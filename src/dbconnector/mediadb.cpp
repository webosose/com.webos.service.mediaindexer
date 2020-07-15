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
    LOG_INFO(0, "Received response com.webos.service.db for: '%s'", method.c_str());

    // handle the media data exists case
    if (method == std::string("find")) {
        if (!sd.object) {
            LOG_DEBUG("sd.object is invalid");
            return false;
        }

        MediaItemPtr mi(static_cast<MediaItem *>(sd.object));

        // we do not need to check, the service implementation should do that
        pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());
        const char *payload = LSMessageGetPayload(msg);
        LOG_DEBUG("payload : %s, method : %s", payload, method.c_str());

        if (!parser.parse(payload)) {
            LOG_ERROR(0, "Invalid JSON message: %s", payload);
            return false;
        }

        pbnjson::JValue domTree(parser.getDom());

        // nothing found - this must be a new item which needs further
        // inspections
        if (!domTree.hasKey("results")) {
            LOG_DEBUG("New media item '%s' needs meta data", mi->uri().c_str());
            mi->observer()->metaDataUpdateRequired(std::move(mi));
            return true;
        }

        auto matches = domTree["results"];

        // sanity check
        if (!matches.isArray() || matches.arraySize() == 0) {
            mi->observer()->metaDataUpdateRequired(std::move(mi));
            return true;
        }

        // gotcha
        auto match = matches[0];

        auto uri = match["uri"].asString();
        auto hashStr = match["hash"].asString();
        auto hash = std::stoul(hashStr);

        // if present and complete, remove the dirty flag
        unflagDirty(mi->uri());

        // check if media item has changed since last visited
        if (mi->hash() != hash) {
            LOG_DEBUG("Media item '%s' hash changed, request meta data update",
                mi->uri().c_str());
            mi->observer()->metaDataUpdateRequired(std::move(mi));
        } else {
            LOG_DEBUG("Media item '%s' unchanged", mi->uri().c_str());
        }
    }
    else if (method == std::string("search"))
    {
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
    if (mediaItemMap_.find(uri) != mediaItemMap_.end()) {
        if (mediaItemMap_[uri] != hash) {
            mediaItemMap_[uri] = hash;
            find(mediaItem->uri(), true, mi, "", false);
        }
    } else {
        mediaItemMap_.emplace(std::make_pair(uri, hash));
        find(mediaItem->uri(), true, mi, "", false);
    }

    mediaItem.release();
}

void MediaDb::updateMediaItem(MediaItemPtr mediaItem)
{
    // update or create the device in the database
    auto props = pbnjson::Object();
    props.put(URI, mediaItem->uri());
    props.put(HASH, std::to_string(mediaItem->hash()));
    props.put(DIRTY, false);
    props.put(TYPE, mediaItem->mediaTypeToString(mediaItem->type()));
    props.put(MIME, mediaItem->mime());

    auto typeProps = pbnjson::Object();
    typeProps.put(URI, mediaItem->uri());
    typeProps.put(MIME, mediaItem->mime());

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

void MediaDb::markDirty(std::shared_ptr<Device> device)
{
    // update or create the device in the database
    auto props = pbnjson::Object();
    props.put(DIRTY, true);

    mergePut(device->uri(), false, props);
}

void MediaDb::unflagDirty(const std::string &uri)
{
    // update or create the device in the database
    auto props = pbnjson::Object();
    props.put(DIRTY, false);

    mergePut(uri, true, props);
}

void MediaDb::grantAccess(const std::string &serviceName)
{
    LOG_INFO(0, "Add read-only access to media db for '%s'",
        serviceName.c_str());
    dbClients_.push_back(serviceName);
    roAccess(dbClients_);
}

bool MediaDb::getAudioList(const std::string &uri, pbnjson::JValue &resp)
{
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(MIME);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Title));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Genre));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Album));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Artist));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::AlbumArtist));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Track));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::TotalTracks));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Duration));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));

    return search(AUDIO_KIND, selectArray, URI, uri, false, &resp, true);
}

bool MediaDb::getAudioMetadata(const std::string &uri, pbnjson::JValue &resp)
{
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(MIME);
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
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(MIME);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Duration));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Width));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Height));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));

    return search(VIDEO_KIND, selectArray, URI, uri, false, &resp, true);
}

bool MediaDb::getVideoMetadata(const std::string &uri, pbnjson::JValue &resp)
{
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(MIME);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Duration));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FrameRate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Width));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Height));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));

    return search(VIDEO_KIND, selectArray, URI, uri, true, &resp, true);
}

bool MediaDb::getImageList(const std::string &uri, pbnjson::JValue &resp)
{
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(MIME);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Width));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Height));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));

    return search(IMAGE_KIND, selectArray, URI, uri, false, &resp, true);
}

bool MediaDb::getImageMetadata(const std::string &uri, pbnjson::JValue &resp)
{
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(MIME);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Width));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Height));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::GeoLocCity));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::GeoLocCountry));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::GeoLocLatitude));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::GeoLocLongitude));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));

    return search(IMAGE_KIND, selectArray, URI, uri, true, &resp, true);
}

void MediaDb::makeUriIndex(){
    auto index = pbnjson::Object();
    index.put("name", URI);

    auto props = pbnjson::Array();
    auto prop = pbnjson::Object();
    prop.put("name", URI);
    props << prop;

    index.put("props", props);

    uriIndexes_ << index;
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
