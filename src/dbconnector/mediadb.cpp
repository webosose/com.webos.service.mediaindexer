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
#include "mediaindexer.h"

#include <cstdio>
#include <gio/gio.h>
#include <cstdint>
#include <cstring>
#include <unistd.h>
std::unique_ptr<MediaDb> MediaDb::instance_;

MediaDb *MediaDb::instance()
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::instance()");
    if (!instance_.get()) {
        instance_.reset(new MediaDb);
        //instance_->ensureKind();
        instance_->ensureKind(AUDIO_KIND);
        instance_->ensureKind(VIDEO_KIND);
        instance_->ensureKind(IMAGE_KIND);
    }
    return instance_.get();
}

MediaDb::~MediaDb()
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::~MediaDb()");
    mediaItemMap_.clear();
}

bool MediaDb::handleLunaResponse(LSMessage *msg)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::handleLunaResponse");
    struct SessionData sd;
    LSMessageToken token = LSMessageGetResponseToken(msg);
    
    if (!sessionDataFromToken(token, &sd)) {
        LOG_ERROR(0, "Failed to find session data from message token %ld", (long)token);
        return false;
    }

    auto method = sd.dbServiceMethod;
    LOG_INFO(0, "[OYJ_DBG] Received response com.webos.mediadb for: '%s'", method.c_str());

    // handle the media data exists case
    if (method == std::string("find") ||
        method == std::string("putPermissions")) {
        if (!sd.object) {
            LOG_ERROR(0, "Invalid object in session data");
            return false;
        }
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
        if (!sd.object) {
            LOG_ERROR(0, "Search should include SessionData");
            return false;
        }
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
        pbnjson::JValue array = pbnjson::Array();
        if (domTree.hasKey("results")){
            auto matches = domTree["results"];
            if (matches.isArray() && matches.isValid() && !matches.isNull())
                reply->put("results", matches);
            else
                reply->put("results", array);
        } else
            reply->put("results", array);

        LOG_DEBUG("search response payload : %s",payload);
    } else if (method == std::string("unflagDirty") ||
                                              method == std::string("mergePut")) {
        LOG_DEBUG("method : %s", method.c_str());
        if (sd.object) {
            MediaItemWrapper_t *miw = static_cast<MediaItemWrapper_t *>(sd.object);
            if (!miw || !miw->mediaItem_) {
                LOG_DEBUG("No MediaItemPtr Found");
                return true;
            }
            MediaItemPtr mi = std::move(miw->mediaItem_);
            DevicePtr device = mi->device();
            if (device) {
                device->incrementProcessedItemCount(mi->type());
                if (device->processingDone()) {
                    LOG_DEBUG("Activate cleanup task");
                    device->activateCleanUpTask();
                }                
            }
            free(miw);
        }
    } else if (method == std::string("del")) {
        if (!sd.object) {
            LOG_ERROR(0, "Search should include SessionData");
            return false;
        }

        pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());
        const char *payload = LSMessageGetPayload(msg);

        if (!parser.parse(payload)) {
            LOG_ERROR(0, "Invalid JSON message: %s", payload);
            return false;
        }
        LOG_DEBUG("del response payload : %s",payload);

        pbnjson::JValue domTree(parser.getDom());
        // response message
        auto reply = static_cast<pbnjson::JValue *>(sd.object);
        *reply = domTree;
    }
    return true;
}

//TODO : handleLunaResponse should be merged including above handler.
bool MediaDb::handleLunaResponseMetaData(LSMessage *msg)
{
    struct SessionData sd;
    LSMessageToken token = LSMessageGetResponseToken(msg);
    
    if (!sessionDataFromToken(token, &sd)) {
        LOG_ERROR(0, "Failed to find session data from message token %ld", (long)token);
        return false;
    }

    auto dbServiceMethod = sd.dbServiceMethod;
    auto dbMethod = sd.dbMethod;
    auto dbQuery = sd.query;
    auto object = sd.object;
    LOG_INFO(0, "[OYJ_DBG] Received response com.webos.mediadb for: method[%s], \
            dbMethod[%s]", dbServiceMethod.c_str(), dbMethod.c_str());

    if (dbServiceMethod == std::string("search")) {
        // we do not need to check, the service implementation should do that
        pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());
        const char *payload = LSMessageGetPayload(msg);

        if (!parser.parse(payload)) {
            LOG_ERROR(0, "Invalid JSON message: %s", payload);
            return false;
        }

        pbnjson::JValue domTree(parser.getDom());
        auto result = domTree["results"];

        MediaIndexer *indexer = MediaIndexer::instance();
        indexer->sendMediaMetaDataNotification(dbMethod, result.stringify(),
                static_cast<LSMessage*>(object));

        // again send search command if payload has "next" key.
        // object null means subscription
        if (domTree.hasKey("next") && object == nullptr) {
            std::string page = domTree["next"].asString();
            dbQuery.put("page", page);

            bool ret = search(dbQuery, dbMethod, object);
            return ret;
        }
    }
    return true;
}

void MediaDb::checkForChange(MediaItemPtr mediaItem)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::checkForChange");
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
    LOG_INFO(0, "[OYJ_DBG] MediaDb::needUpdate");
    bool ret = false;
    if (!mediaItem) {
        LOG_ERROR(0, "Invalid input");
        return false;
    }
    pbnjson::JValue resp = pbnjson::Object();
    std::string kind = "";
    if (mediaItem->type() != MediaItem::Type::EOL)
        kind = kindMap_[mediaItem->type()];
    do {
        ret = find(mediaItem->uri(), true, &resp, kind, true);
    } while(!ret);

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
        LOG_DEBUG("Media item '%s' has some missing information, need to be updated", mediaItem->uri().c_str());
        return true;
    } else {
        LOG_DEBUG("Media item '%s' unchanged", mediaItem->uri().c_str());
    }

    LOG_DEBUG("Media item '%s' doesn't need to be changed", mediaItem->uri().c_str());
    return false;
}

bool MediaDb::isEnoughInfo(MediaItem *mediaItem, pbnjson::JValue &val)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::isEnoughInfo");
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
    LOG_INFO(0, "[OYJ_DBG] MediaDb::updateMediaItem");
    LOG_DEBUG("%s Start for mediaItem uri : %s",__FUNCTION__, mediaItem->uri().c_str());
    // update or create the device in the database
    if (mediaItem->type() == MediaItem::Type::EOL) {
        LOG_ERROR(0, "Invalid media type");
        return;
    }
    auto typeProps = pbnjson::Object();
    typeProps.put(URI, mediaItem->uri());
    typeProps.put(HASH, std::to_string(mediaItem->hash()));
    typeProps.put(DIRTY, false);
    //typeProps.put(TYPE, mediaItem->mediaTypeToString(mediaItem->type()));
    //typeProps.put(MIME, mediaItem->mime());
    auto filepath = getFilePath(mediaItem->uri());
    typeProps.put(FILE_PATH, filepath ? filepath.value() : "");

    std::string kind_type = kindMap_[mediaItem->type()];

    for (auto meta = MediaItem::Meta::Title; meta < MediaItem::Meta::EOL; ++meta) {
        auto metaStr = mediaItem->metaToString(meta);
        auto data = mediaItem->meta(meta);

        //Todo - Only media kind columns should be stored.
        //if(mediaItem->isMediaMeta(meta))
        //mediaItem->putProperties(metaStr, data, props);

        if ((mediaItem->type() == MediaItem::Type::Audio && mediaItem->isAudioMeta(meta))
            ||(mediaItem->type() == MediaItem::Type::Video && mediaItem->isVideoMeta(meta))
            ||(mediaItem->type() == MediaItem::Type::Image && mediaItem->isImageMeta(meta))) {
            mediaItem->putProperties(metaStr, data, typeProps);
        }
    }
    //mergePut(mediaItem->uri(), true, props, nullptr, MEDIA_KIND);
    auto uri = mediaItem->uri();
    MediaItemWrapper_t *mi = new MediaItemWrapper_t;
    mi->mediaItem_ = std::move(mediaItem);
    mergePut(uri, true, typeProps, mi, kind_type);
}

std::optional<std::string> MediaDb::getFilePath(
    const std::string &uri) const
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::getFilePath");
    // check if the plugin is available and get it
    auto plg = PluginFactory().plugin(uri);
    if (!plg)
        return std::nullopt;

    return plg->getPlaybackUri(uri);
}

void MediaDb::markDirty(std::shared_ptr<Device> device, MediaItem::Type type)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::markDirty");
    // update or create the device in the database
    auto props = pbnjson::Object();
    props.put(DIRTY, true);

    //mergePut(device->uri(), false, props);
    if (type == MediaItem::Type::EOL) {
        merge(AUDIO_KIND, props, URI, device->uri(), false);
        merge(VIDEO_KIND, props, URI, device->uri(), false);
        merge(IMAGE_KIND, props, URI, device->uri(), false);
    } else
        merge(kindMap_[type], props, URI, device->uri(), false);
}

void MediaDb::unflagDirty(MediaItemPtr mediaItem)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::unflagDirty");
    // update or create the device in the database
    auto props = pbnjson::Object();
    props.put(DIRTY, false);
    std::string uri = mediaItem->uri();
    MediaItem::Type type = mediaItem->type();

    //mergePut(uri, true, props);
    if (type != MediaItem::Type::EOL) {
        MediaItemWrapper_t *mi = new MediaItemWrapper_t;
        mi->mediaItem_ = std::move(mediaItem);
        merge(kindMap_[type], props, URI, uri, true, mi, false, "unflagDirty");
    } else {
        LOG_ERROR(0, "ERROR : Media Item type for uri %s should not be EOL", uri.c_str());
    }
}

void MediaDb::removeDirty(Device* device)
{
    /*
    LOG_INFO(0, "[OYJ_DBG] MediaDb::removeDirty");
    std::map<MediaItem::Type, pbnjson::JValue> listMap;
    std::string uri = device->uri();

    auto selectArray = pbnjson::Array();
    selectArray.append(MediaItem::metaToString(MediaItem::CommonType::URI));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));

    auto where = prepareWhere(URI, uri, false);
    auto filter = prepareWhere(DIRTY, true, true);
   
    std::string method = std::string("removeDirty");
    for (auto const &[type, kind] : kindMap_) {
        listMap[type] = pbnjson::Object();
        search(kind, selectArray, where, filter, &listMap[type], true, method);
        LOG_DEBUG("listMap result for %s : %s",MediaItem::mediaTypeToString(type).c_str(), listMap[type].stringify().c_str());
        if (listMap[type].hasKey("results")) {
            auto rmlist = listMap[type]["results"];
            if (rmlist.isArray() && rmlist.isValid() && !rmlist.isNull()) {
                for (auto element : rmlist.items()) {
                    auto uri_ = element["uri"].asString();
                    auto thumbnail_ = element["thumbnail"].asString();
                    if (!uri_.empty()) {
                        auto where = prepareWhere(URI, element["uri"].asString(), true);
                        del(kind, where);
                    }
                    if (!thumbnail_.empty()) {
                        std::remove(thumbnail_.c_str());
                        sync();
                    }
                }
            }
        }
    }
    */
}

void MediaDb::grantAccess(const std::string &serviceName)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::grantAccess");
    LOG_INFO(0, "Add read-only access to media db for '%s'",
        serviceName.c_str());
    dbClients_.push_back(serviceName);
    roAccess(dbClients_);
}

void MediaDb::grantAccessAll(const std::string &serviceName, bool atomic, pbnjson::JValue &resp)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::grantAccessAll");
    LOG_INFO(0, "Add read-only access to media db for '%s'",
        serviceName.c_str());
    dbClients_.push_back(serviceName);
    std::list<std::string> kindList_ = {AUDIO_KIND, VIDEO_KIND, IMAGE_KIND};
    if (atomic)
        roAccess(dbClients_, kindList_, &resp, atomic);
    else
        roAccess(dbClients_, kindList_, nullptr, atomic);
}

bool MediaDb::getAudioList(const std::string &uri, int count, LSMessage *msg)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::getAudioList");
    LOG_DEBUG("%s Start for uri : %s", __func__, uri.c_str());
    auto selectArray = pbnjson::Array();
    selectArray.append(MediaItem::metaToString(MediaItem::CommonType::URI));
    selectArray.append(MediaItem::metaToString(MediaItem::CommonType::FILEPATH));
    selectArray.append(MediaItem::metaToString(MediaItem::CommonType::DIRTY));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Genre));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Album));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Artist));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::LastModifiedDate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Title));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Duration));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));

    auto where = pbnjson::Object();
    auto filter = pbnjson::Object();
    if (uri.empty()) {
        where = prepareWhere(DIRTY, false, true);
    } else {
        where = prepareWhere(URI, uri, false);
        filter = prepareWhere(DIRTY, false, true);
    }

    auto query = pbnjson::Object();
    query.put("select", selectArray);
    query.put("from", AUDIO_KIND);
    query.put("where", where);
    if(filter.isArray() && filter.arraySize() > 0)
        query.put("filter", filter);

    if (count != 0)
        query.put("limit", count);
    
    std::string dbMethod = std::string("getAudioList");
    return search(query, dbMethod, msg);
}

bool MediaDb::getVideoList(const std::string &uri, int count, LSMessage *msg)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::getVideoList");
    LOG_DEBUG("%s Start for uri : %s", __func__, uri.c_str());
    auto selectArray = pbnjson::Array();
    selectArray.append(MediaItem::metaToString(MediaItem::CommonType::URI));
    selectArray.append(MediaItem::metaToString(MediaItem::CommonType::FILEPATH));
    selectArray.append(MediaItem::metaToString(MediaItem::CommonType::DIRTY));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::LastModifiedDate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Width));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Height));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Title));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Duration));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Thumbnail));

    auto where = pbnjson::Object();
    auto filter = pbnjson::Object();
    if (uri.empty()) {
        where = prepareWhere(DIRTY, false, true);
    } else {
        where = prepareWhere(URI, uri, false);
        filter = prepareWhere(DIRTY, false, true);
    }

    std::string method = std::string("getVideoList");
//    return search(query, dbMethod, msg);
//    return search(VIDEO_KIND, selectArray, where, filter, &resp, true, method);
    return true;
}

bool MediaDb::getImageList(const std::string &uri, int count, LSMessage *msg)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::getImageList");
    LOG_DEBUG("%s Start for uri : %s", __FUNCTION__, uri.c_str());
    auto selectArray = pbnjson::Array();
    selectArray.append(URI);
    selectArray.append(TYPE);
    selectArray.append(MediaItem::metaToString(MediaItem::CommonType::DIRTY));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::LastModifiedDate));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::FileSize));
    selectArray.append(FILE_PATH);
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Title));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Width));
    selectArray.append(MediaItem::metaToString(MediaItem::Meta::Height));

    auto where = pbnjson::Object();
    auto filter = pbnjson::Object();
    if (uri.empty()) {
        where = prepareWhere(DIRTY, false, true);
    } else {
        where = prepareWhere(URI, uri, false);
        filter = prepareWhere(DIRTY, false, true);
    }

    std::string method = std::string("getImageList");
    return true;
//    return search(IMAGE_KIND, selectArray, where, filter, &resp, true, method);
}

bool MediaDb::requestDelete(const std::string &uri, pbnjson::JValue &resp)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::requestDelete");
    LOG_DEBUG("%s Start for uri : %s", __FUNCTION__, uri.c_str());
    auto where = prepareWhere(URI, uri, true);
    MediaItem::Type type =guessType(uri);
    return del(kindMap_[type], where, &resp, true);
}

MediaItem::Type MediaDb::guessType(const std::string &uri)
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::guessType");
    LOG_DEBUG("%s Start for uri : %s", __FUNCTION__, uri.c_str());
    gchar *contentType = NULL;
    gboolean uncertain;

    contentType = g_content_type_guess(uri.c_str(), NULL, 0, &uncertain);
    if (contentType) {
        MediaItem::Type type = MediaItem::typeFromMime(contentType);
        return type;
    }
    g_free(contentType);
    return MediaItem::Type::EOL;
}

pbnjson::JValue MediaDb::prepareWhere(const std::string &key,
                                                 const std::string &value,
                                                 bool precise,
                                                 pbnjson::JValue whereClause) const
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::prepareWhere(key, string, precise, whereClause");
    auto cond = pbnjson::Object();
    cond.put("prop", key);
    cond.put("op", precise ? "=" : "%");
    cond.put("val", value);
    whereClause << cond;
    return whereClause;
}

pbnjson::JValue MediaDb::prepareWhere(const std::string &key,
                                                 bool value,
                                                 bool precise,
                                                 pbnjson::JValue whereClause) const
{
    LOG_INFO(0, "[OYJ_DBG] MediaDb::prepareWhere(key, value, precise, whereClause");
    auto cond = pbnjson::Object();
    cond.put("prop", key);
    cond.put("op", precise ? "=" : "%");
    cond.put("val", value);
    whereClause << cond;
    return whereClause;
}

void MediaDb::makeUriIndex(){
    LOG_INFO(0, "[OYJ_DBG] MediaDb::makeUriIndex");
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
    LOG_INFO(0, "[OYJ_DBG] MediaDb::MediaDb()");
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
