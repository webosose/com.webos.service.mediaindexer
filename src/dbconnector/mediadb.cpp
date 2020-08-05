// Copyright (c) 2019-2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "mediadb.h"
#include "mediaitem.h"
#include "imediaitemobserver.h"
#include "device.h"
#include "plugins/pluginfactory.h"
#include "plugins/plugin.h"

#include <cstdint>

std::unique_ptr<MediaDb> MediaDb::instance_;
std::mutex MediaDb::ctorLock_;


MediaDb *MediaDb::instance()
{
    std::lock_guard<std::mutex> lk(ctorLock_);
    if (!instance_.get()) {
        instance_.reset(new MediaDb);
        instance_->ensureKind();
        instance_->ensureKind(AUDIO_KIND);
        instance_->ensureKind(VIDEO_KIND);
        instance_->ensureKind(IMAGE_KIND);
    }
    return instance_.get();
}

MediaDb::~MediaDb()
{
    mediaItemMap_.clear();
}

bool MediaDb::handleLunaResponse(LSMessage *msg)
{
    struct SessionData sd;

    if (!sessionDataFromToken(LSMessageGetResponseToken(msg), &sd))
        return false;

    auto method = sd.method;
    LOG_INFO(0, "Received response com.webos.mediadb for: '%s'", method.c_str());

    // handle the media data exists case
    if (method == std::string("find") ||
        method == std::string("putPermissions")) {
        if (!sd.object)
            return false;
        // we do not need to check, the service implementation should do that
        pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());
        const char *payload = LSMessageGetPayload(msg);

        if (!parser.parse(payload)) {
            LOG_ERROR(0, "Invalid JSON message: %s", payload);
            return false;
        }
        LOG_DEBUG("payload : %s",payload);

        pbnjson::JValue domTree(parser.getDom());
        // response message
        auto reply = static_cast<pbnjson::JValue *>(sd.object);
        *reply = domTree;

    } else if (method == std::string("search")) {
        if (!sd.object)
            return false;
        // we do not need to check, the service implementation should do that
        pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());
        const char *payload = LSMessageGetPayload(msg);

        if (!parser.parse(payload)) {
            LOG_ERROR(0, "Invalid JSON message: %s", payload);
            return false;
        }

        pbnjson::JValue domTree(parser.getDom());
        // response message
        auto reply = static_cast<pbnjson::JValue *>(sd.object);

        if (!domTree.hasKey("results")){
            return false;
        }

        // response message
        auto matches = domTree["results"];
        reply->put("results", matches);

        LOG_DEBUG("search response payload : %s",payload);
    }
    return true;
}

void MediaDb::checkForChange(MediaItemPtr mediaItem)
{
    auto mi = mediaItem.get();
    auto uri = mediaItem->uri();
    auto hash = mediaItem->hash();
    if ((mediaItemMap_.find(uri) == mediaItemMap_.end()) ||
        ((mediaItemMap_.find(uri) != mediaItemMap_.end()) && (mediaItemMap_[uri] != hash))) {
        mediaItemMap_[uri] = hash;
        find(mediaItem->uri(), true, mi, "", false);
    }

    mediaItem.release();
}

bool MediaDb::needUpdate(MediaItem *mediaItem)
{
    bool ret = false;
    if (!mediaItem) {
        LOG_ERROR(0, "Invalid input");
        return false;
    }
    pbnjson::JValue resp = pbnjson::Object();
    std::string kind = "";
    if (mediaItem->type() != MediaItem::Type::EOL)
        kind = kindMap_[mediaItem->type()];
    find(mediaItem->uri(), true, &resp, kind, true);

    LOG_DEBUG("find result for %s : %s",mediaItem->uri().c_str(), resp.stringify().c_str());

    if (!resp.hasKey("results")) {
        LOG_DEBUG("New media item '%s' needs meta data", mediaItem->uri().c_str());
        return true;
    }

    auto matches = resp["results"];

    // sanity check
    if (!matches.isArray() || matches.arraySize() == 0) {
        return true;
    }

    // gotcha
    auto match = matches[0];

    if (!match.hasKey("uri") || !match.hasKey("hash")) {
        LOG_DEBUG("Current db data is insufficient, need update");
        return true;
    }

    auto uri = match["uri"].asString();
    auto hashStr = match["hash"].asString();
    auto hash = std::stoul(hashStr);

    // check if media item has changed since last visited
    if (mediaItem->hash() != hash) {
        LOG_DEBUG("Media item '%s' hash changed, request meta data update",
            mediaItem->uri().c_str());
        return true;
    } else if (!isEnoughInfo(mediaItem, match)) {
        LOG_DEBUG("Media item '%s' has some missing information, need to be updated");
        return true;
    } else {
        LOG_DEBUG("Media item '%s' unchanged", mediaItem->uri().c_str());
    }

    LOG_DEBUG("Media item '%s' doesn't need to be changed", mediaItem->uri().c_str());
    return ret;
}

bool MediaDb::isEnoughInfo(MediaItem *mediaItem, pbnjson::JValue &val)
{
    bool enough = false;
    if (!mediaItem) {
        LOG_ERROR(0, "Invalid input");
        return false;
    }
    MediaItem::Type type = mediaItem->type();

    switch (type) {
        case MediaItem::Type::Audio:
        case MediaItem::Type::Video:
            if (val.hasKey("thumbnail") && !val["thumbnail"].asString().empty())
                enough = true;
            break;
        case MediaItem::Type::Image:
            if (val.hasKey("width") && val.hasKey("height") &&
                !val["width"].asString().empty() && !val["height"].asString().empty())
                enough = true;
            break;
        default:
            break;
    }
    return enough;
}

void MediaDb::updateMediaItem(MediaItemPtr mediaItem)
{
    LOG_DEBUG("%s Start for mediaItem uri : %s",__FUNCTION__, mediaItem->uri().c_str());
    // update or create the device in the database
    auto props = pbnjson::Object();
    props.put(URI, mediaItem->uri());
    props.put(HASH, std::to_string(mediaItem->hash()));
    props.put(DIRTY, false);
    props.put(TYPE, mediaItem->mediaTypeToString(mediaItem->type()));
    props.put(MIME, mediaItem->mime());

    auto typeProps = pbnjson::Object();
    typeProps.put(URI, mediaItem->uri());
    typeProps.put(HASH, std::to_string(mediaItem->hash()));
    typeProps.put(DIRTY, false);
    typeProps.put(TYPE, mediaItem->mediaTypeToString(mediaItem->type()));
    typeProps.put(MIME, mediaItem->mime());
    auto filepath = getFilePath(mediaItem->uri());
    typeProps.put(FILE_PATH, filepath ? filepath.value() : "");

    std::string kind_type;
    switch (mediaItem->type()) {
        case MediaItem::Type::Audio:
            kind_type = AUDIO_KIND;
            break;
        case MediaItem::Type::Video:
            kind_type = VIDEO_KIND;
            break;
        case MediaItem::Type::Image:
            kind_type = IMAGE_KIND;
            break;
        default:
            break;
    }

    for (auto meta = MediaItem::Meta::Title; meta < MediaItem::Meta::EOL; ++meta) {
        auto metaStr = mediaItem->metaToString(meta);
        auto data = mediaItem->meta(meta);

        //Todo - Only media kind columns should be stored.
        //if(mediaItem->isMediaMeta(meta))
        props = putProperties(metaStr, data, props);

        if ((mediaItem->type() == MediaItem::Type::Audio && mediaItem->isAudioMeta(meta))
            ||(mediaItem->type() == MediaItem::Type::Video && mediaItem->isVideoMeta(meta))
            ||(mediaItem->type() == MediaItem::Type::Image && mediaItem->isImageMeta(meta))) {
            typeProps = putProperties(metaStr, data, typeProps);
        }
    }
    mergePut(mediaItem->uri(), true, props, nullptr, MEDIA_KIND);
    mergePut(mediaItem->uri(), true, typeProps, nullptr, kind_type);
}

pbnjson::JValue MediaDb::putProperties(std::string metaStr, std::optional<MediaItem::MetaData> data, pbnjson::JValue &props)
{
    if (data.has_value()) {
        auto content = data.value();
        switch (content.index()) {
            case 0:
                props.put(metaStr, std::get<std::int64_t>(content));
                break;
            case 1:
                props.put(metaStr, std::get<double>(content));
                break;
            case 2:
                props.put(metaStr, std::to_string(std::get<std::int32_t>(content)));
                break;
            case 3:
                props.put(metaStr, std::get<std::string>(content));
                break;
            case 4:
                props.put(metaStr, std::to_string(std::get<std::uint32_t>(content)));
                break;
        }
    } else {
        props.put(metaStr, std::string(""));
    }
    return props;
}

std::optional<std::string> MediaDb::getFilePath(
    const std::string &uri) const
{
    // check if the plugin is available and get it
    auto plg = PluginFactory().plugin(uri);
    if (!plg)
        return std::nullopt;

    return plg->getPlaybackUri(uri);
}

void MediaDb::markDirty(std::shared_ptr<Device> device)
{
    // update or create the device in the database
    auto props = pbnjson::Object();
    props.put(DIRTY, true);

    mergePut(device->uri(), false, props);
    merge(AUDIO_KIND, props, URI, device->uri(), false);
    merge(VIDEO_KIND, props, URI, device->uri(), false);
    merge(IMAGE_KIND, props, URI, device->uri(), false);
}

void MediaDb::unflagDirty(const std::string &uri)
{
    // update or create the device in the database
    auto props = pbnjson::Object();
    props.put(DIRTY, false);

    mergePut(uri, true, props);
    merge(AUDIO_KIND, props, URI, uri, true);
    merge(VIDEO_KIND, props, URI, uri, true);
    merge(IMAGE_KIND, props, URI, uri, true);
}

void MediaDb::grantAccess(const std::string &serviceName)
{
    LOG_INFO(0, "Add read-only access to media db for '%s'",
        serviceName.c_str());
    dbClients_.push_back(serviceName);
    roAccess(dbClients_);
}

void MediaDb::grantAccessAll(const std::string &serviceName, pbnjson::JValue &resp)
{
    LOG_INFO(0, "Add read-only access to media db for '%s'",
        serviceName.c_str());
    dbClients_.push_back(serviceName);
    std::list<std::string> kindList_ = {AUDIO_KIND, VIDEO_KIND, IMAGE_KIND};
    roAccess(dbClients_, kindList_, &resp);
}


bool MediaDb::getAudioList(const std::string &uri, pbnjson::JValue &resp)
{
    LOG_DEBUG("%s Start for uri : %s", __FUNCTION__, uri.c_str());
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(TYPE);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::LastModifiedDate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));
    selectArray.append(FILE_PATH);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Title));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Duration));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));

    return search(AUDIO_KIND, selectArray, URI, uri, false, &resp, true);
}

bool MediaDb::getAudioMetadata(const std::string &uri, pbnjson::JValue &resp)
{
    LOG_DEBUG("%s Start for uri : %s", __FUNCTION__, uri.c_str());
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(MIME);
    selectArray.append(TYPE);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::DateOfCreation));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::LastModifiedDate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));
    selectArray.append(FILE_PATH);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Title));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Genre));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Album));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Artist));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::AlbumArtist));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Track));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::TotalTracks));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Duration));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::SampleRate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::BitPerSample));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::BitRate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Channels));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Lyric));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));

    return search(AUDIO_KIND, selectArray, URI, uri, true, &resp, true);
}

bool MediaDb::getVideoList(const std::string &uri, pbnjson::JValue &resp)
{
    LOG_DEBUG("%s Start for uri : %s", __FUNCTION__, uri.c_str());
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(TYPE);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::LastModifiedDate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));
    selectArray.append(FILE_PATH);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Title));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Duration));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));

    return search(VIDEO_KIND, selectArray, URI, uri, false, &resp, true);
}

bool MediaDb::getVideoMetadata(const std::string &uri, pbnjson::JValue &resp)
{
    LOG_DEBUG("%s Start for uri : %s", __FUNCTION__, uri.c_str());
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(MIME);
    selectArray.append(TYPE);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::DateOfCreation));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::LastModifiedDate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));
    selectArray.append(FILE_PATH);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Title));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Duration));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Width));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Height));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FrameRate));

    return search(VIDEO_KIND, selectArray, URI, uri, true, &resp, true);
}

bool MediaDb::getImageList(const std::string &uri, pbnjson::JValue &resp)
{
    LOG_DEBUG("%s Start for uri : %s", __FUNCTION__, uri.c_str());
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(TYPE);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::LastModifiedDate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));
    selectArray.append(FILE_PATH);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Title));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Width));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Height));

    return search(IMAGE_KIND, selectArray, URI, uri, false, &resp, true);
}

bool MediaDb::getImageMetadata(const std::string &uri, pbnjson::JValue &resp)
{
    LOG_DEBUG("%s Start for uri : %s", __FUNCTION__, uri.c_str());
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(MIME);
    selectArray.append(TYPE);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::DateOfCreation));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::LastModifiedDate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));
    selectArray.append(FILE_PATH);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Title));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Width));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Height));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::GeoLocCity));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::GeoLocCountry));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::GeoLocLatitude));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::GeoLocLongitude));

    return search(IMAGE_KIND, selectArray, URI, uri, true, &resp, true);
}

void MediaDb::makeUriIndex(){
    std::list<std::string> indexes = {URI, DIRTY};
    for (auto idx : indexes) {
        auto index = pbnjson::Object();
        index.put("name", idx);

        auto props = pbnjson::Array();
        auto prop = pbnjson::Object();
        prop.put("name", idx);
        props << prop;

        index.put("props", props);

        uriIndexes_ << index;
    }
}

MediaDb::MediaDb() :
    DbConnector("com.webos.service.mediaindexer.media", true)
{
    std::list<std::string> indexes = {URI, TYPE};
    for (auto idx : indexes) {
        auto index = pbnjson::Object();
        index.put("name", idx);

        auto props = pbnjson::Array();
        auto prop = pbnjson::Object();
        prop.put("name", idx);
        props << prop;

        index.put("props", props);

        kindIndexes_ << index;
    }
    makeUriIndex();
}
