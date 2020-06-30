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

MediaDb *MediaDb::instance()
{
    if (!instance_.get()) {
        instance_.reset(new MediaDb);
        instance_->ensureKind();
    }
    return instance_.get();
}

MediaDb::~MediaDb()
{
    // nothing to be done here
}

bool MediaDb::setResponseDestination(const std::string &methodKey, LSHandle *hdl, LSMessage *msg)
{
    std::lock_guard<std::mutex> lk(lock_);
    currMethod_ = methodKey;

    respDest_.insert_or_assign(currMethod_, std::make_pair(hdl, msg));
    LSMessageRef(reinterpret_cast<LSMessage*>(msg));
    return true;
}

bool MediaDb::sendResponseToDestination(LSHandle *hdl, LSMessage *dst, const char *message)
{
    if (!dst)
    {
        LOG_ERROR(0, "Invalid LSMessage");
        return false;
    }
    if (!hdl)
    {
        LOG_ERROR(0, "LSHandle extracted from msg is invalid");
        return false;
    }

    LSError lsError;
    LSErrorInit(&lsError);
    if (!LSMessageReply(hdl, dst, message, &lsError))
    {
        LOG_ERROR(0, "Message reply error");
        return false;
    }
    LSMessageUnref(reinterpret_cast<LSMessage*>(dst));
    return true;
}

bool MediaDb::handleLunaResponse(LSMessage *msg)
{
    struct SessionData sd;

    if (!sessionDataFromToken(LSMessageGetResponseToken(msg), &sd))
        return false;

    auto method = sd.method;
    LOG_INFO(0, "Received response com.webos.service.db for: '%s'",
        method.c_str());

    // handle the media data exists case
    if (method == std::string("find")) {
        if (!sd.object)
            return false;

        MediaItemPtr mi(static_cast<MediaItem *>(sd.object));

        // we do not need to check, the service implementation should do that
        pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());
        const char *payload = LSMessageGetPayload(msg);

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
    }else if (method == std::string("search")) {

        // we do not need to check, the service implementation should do that
        pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());
        const char *payload = LSMessageGetPayload(msg);

        if (!parser.parse(payload)) {
            LOG_ERROR(0, "Invalid JSON message: %s", payload);
            return false;
        }

        pbnjson::JValue domTree(parser.getDom());
        // response message
        auto reply = pbnjson::Object();

        if (!domTree.hasKey("results")){
            return false;
        }

        // response message
        auto matches = domTree["results"];
        reply.put("results", matches);

        std::lock_guard<std::mutex> lk(lock_);
        if (!currMethod_.empty())
        {
            if (respDest_.find(currMethod_) != respDest_.end())
            {
                auto dst = respDest_[currMethod_];
                LOG_DEBUG("current Method : %s", currMethod_.c_str());
                auto resp = pbnjson::Object();
                resp.put("returnValue", true);
                resp.put("count", matches.arraySize());
                resp.put("results",matches);
                sendResponseToDestination(dst.first, dst.second, resp.stringify().c_str());
            }
            else
                LOG_ERROR(0, "LSMessage information for setting desination is not enough for method %s", currMethod_.c_str());
            currMethod_ = "";
        }
/*
        std::string ret = reply.stringify();
        if (sendNotification(getHandle(), ret, "getAudioList"))
        {
            LOG_DEBUG("Send Notification succeeded");
        }
*/
        LOG_DEBUG("search response payload : %s",payload);

    }

    return true;
}

void MediaDb::checkForChange(MediaItemPtr mediaItem)
{
    auto mi = mediaItem.get();
    find(mediaItem->uri(), true, mi);
    mediaItem.release();
}

void MediaDb::updateMediaItem(MediaItemPtr mediaItem)
{
    // update or create the device in the database
    auto props = pbnjson::Object();
    props.put("uri", mediaItem->uri());
    props.put("hash", std::to_string(mediaItem->hash()));
    props.put("dirty", false);
    props.put("type", mediaItem->mediaTypeToString(mediaItem->type()));
    props.put("mime", mediaItem->mime());

    for (auto meta = MediaItem::Meta::Title;
         meta < MediaItem::Meta::EOL; ++meta) {
        auto metaStr = mediaItem->metaToString(meta);
        auto data = mediaItem->meta(meta);
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
    }

    mergePut(mediaItem->uri(), true, props);
}

void MediaDb::markDirty(std::shared_ptr<Device> device)
{
    // update or create the device in the database
    auto props = pbnjson::Object();
    props.put("dirty", true);

    mergePut(device->uri(), false, props);
}

void MediaDb::unflagDirty(const std::string &uri)
{
    // update or create the device in the database
    auto props = pbnjson::Object();
    props.put("dirty", false);

    mergePut(uri, true, props);
}

void MediaDb::grantAccess(const std::string &serviceName)
{
    LOG_INFO(0, "Add read-only access to media db for '%s'",
        serviceName.c_str());
    dbClients_.push_back(serviceName);
    roAccess(dbClients_);
}

bool MediaDb::getAudioList(const std::string &uri)
{
    LOG_DEBUG("getAudioList from DB");
    return search(uri, false);
}

bool MediaDb::getAudioMetadata(const std::string &uri)
{
    LOG_DEBUG("getAudioMetadata from DB");
    return search(uri, true);
}

bool MediaDb::getVideoList(const std::string &uri)
{
    LOG_DEBUG("getVideoList from DB");
    return search(uri, false);
}

MediaDb::MediaDb() :
    DbConnector("com.webos.service.mediaindexer.media:1")
{
    std::list<std::string> indexes = {"uri", "type"};
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
}
