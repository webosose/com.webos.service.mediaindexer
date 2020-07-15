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

#include "mediaindexer.h"
#include "plugins/pluginfactory.h"
#include "pdmlistener/pdmlistener.h"
#include "dbconnector/dbconnector.h"
#include "mediaitem.h"
#include "dbconnector/mediadb.h"

#include <glib.h>
#include <pbnjson.hpp>

#include <algorithm>

#define RETURN_IF(exp,rv,format,args...) \
        { if(exp) { \
            LOG_ERROR(0, format, ##args); \
            return rv; \
        } \
        }


/// From main.cpp.
extern const char *lunaServiceId;

LSMethod IndexerService::serviceMethods_[] = {
    { "runDetect", IndexerService::onRun, LUNA_METHOD_FLAGS_NONE },
    { "stopDetect", IndexerService::onStop, LUNA_METHOD_FLAGS_NONE },
    { "getPlugin", IndexerService::onPluginGet, LUNA_METHOD_FLAGS_NONE },
    { "putPlugin", IndexerService::onPluginPut, LUNA_METHOD_FLAGS_NONE },
    { "getPluginList", IndexerService::onPluginListGet, LUNA_METHOD_FLAGS_NONE },
    { "getDeviceList", IndexerService::onDeviceListGet, LUNA_METHOD_FLAGS_NONE },
    { "getPlaybackUri", IndexerService::onGetPlaybackUri, LUNA_METHOD_FLAGS_NONE },
    { "getAudioList", IndexerService::onGetAudioList, LUNA_METHOD_FLAGS_NONE },
    { "getAudioMetadata", IndexerService::onGetAudioMetadata, LUNA_METHOD_FLAGS_NONE },
    { "getVideoList", IndexerService::onGetVideoList, LUNA_METHOD_FLAGS_NONE },
    { "getVideoMetadata", IndexerService::onGetVideoMetadata, LUNA_METHOD_FLAGS_NONE },
    { "getImageList", IndexerService::onGetImageList, LUNA_METHOD_FLAGS_NONE },
    { "getImageMetadata", IndexerService::onGetImageMetadata, LUNA_METHOD_FLAGS_NONE },
    {NULL, NULL}
};

pbnjson::JSchema IndexerService::pluginGetSchema_(pbnjson::JSchema::fromString(
        "{ \"type\": \"object\","
        "  \"properties\": {"
        "    \"uri\": {"
        "      \"type\": \"string\" }"
        "  }"
        "}"));

pbnjson::JSchema IndexerService::pluginPutSchema_(pbnjson::JSchema::fromString(
        "{ \"type\": \"object\","
        "  \"properties\": {"
        "    \"uri\": {"
        "      \"type\": \"string\" }"
        "  },"
        "  \"required\": [ \"uri\" ]"
        "}"));

pbnjson::JSchema IndexerService::deviceListGetSchema_(
    pbnjson::JSchema::fromString(
        "{ \"type\": \"object\","
        "  \"properties\": {"
        "    \"subscribe\": {"
        "      \"type\": \"boolean\" }"
        "  },"
        "  \"required\": [ \"subscribe\" ]"
        "}"));

pbnjson::JSchema IndexerService::detectRunStopSchema_(
    pbnjson::JSchema::fromString(
        "{ \"type\": \"object\","
        "  \"properties\": {"
        "    \"uri\": {"
        "      \"type\": \"string\" }"
        "  }"
        "}"));

pbnjson::JSchema IndexerService::playbackUriGetSchema_(
    pbnjson::JSchema::fromString(
        "{ \"type\": \"object\","
        "  \"properties\": {"
        "    \"uri\": {"
        "      \"type\": \"string\" }"
        "  },"
        "  \"required\": [ \"uri\" ]"
        "}"));

pbnjson::JSchema IndexerService::metadataGetSchema_(
    pbnjson::JSchema::fromString(
        "{ \"type\": \"object\","
        "  \"properties\": {"
        "    \"uri\": {"
        "      \"type\": \"string\" }"
        "  },"
        "  \"required\": [ \"uri\" ]"
        "}"));


IndexerService::IndexerService(MediaIndexer *indexer) :
    indexer_(indexer)
{
    LSError lsError;
    LSErrorInit(&lsError);

    if (!LSRegister(lunaServiceId, &lsHandle_, &lsError)) {
        LOG_CRITICAL(0, "Unable to register at luna-bus");
        return;
    }

    if (!LSRegisterCategory(lsHandle_, "/", serviceMethods_, NULL, NULL,
            &lsError)) {
        LOG_CRITICAL(0, "Unable to register top level category");
        return;
    }

    if (!LSCategorySetData(lsHandle_, "/", this, &lsError)) {
        LOG_CRITICAL(0, "Unable to set data on top level category");
        return;
    }

    if (!LSGmainAttach(lsHandle_, indexer_->mainLoop_, &lsError)) {
        LOG_CRITICAL(0, "Unable to attach service");
        return;
    }

    /// @todo Implement bus disconnect handler.

    PdmListener::init(lsHandle_);
    DbConnector::init(lsHandle_);

}

IndexerService::~IndexerService()
{
    if (!lsHandle_)
        return;

    LSError lsError;
    LSErrorInit(&lsError);

    if (!LSUnregister(lsHandle_, &lsError))
        LOG_ERROR(0, "Service unregister failed");
}

bool IndexerService::pushDeviceList(LSMessage *msg)
{
    if (msg) {
        // parse incoming message
        const char *payload = LSMessageGetPayload(msg);
        pbnjson::JDomParser parser;

        if (!parser.parse(payload, deviceListGetSchema_)) {
            LOG_ERROR(0, "Invalid getDeviceList request: %s", payload);
            return false;
        }
        LOG_DEBUG("Valid getDeviceList request");

        checkForDeviceListSubscriber(msg, parser);
    }

    // generate response
    auto reply = pbnjson::Object();
    auto pluginList = pbnjson::Array();
    for (auto const &[uri, plg] : indexer_->plugins_) {
        auto plugin = pbnjson::Object();
        plugin.put("active", plg->active());
        plugin.put("uri", uri);

        auto deviceList = pbnjson::Array();
        plg->lock();
        for (auto const &[uri, dev] : plg->devices()) {
            auto device = pbnjson::Object();
            device.put("available", dev->available());
            device.put("uri", uri);

            // now get the meta data
            for (auto type = Device::Meta::Name;
                 type < Device::Meta::EOL; ++type) {
                auto meta = dev->meta(type);
                device.put(Device::metaTypeToString(type), meta);
            }

            // now push the media item count for this device for every
            // given media item type
            for (auto type = MediaItem::Type::Audio;
                 type < MediaItem::Type::EOL; ++type) {
                auto cnt = dev->mediaItemCount(type);
                auto typeStr = MediaItem::mediaTypeToString(type);
                typeStr.append("Count");
                device.put(typeStr, cnt);
            }

            deviceList << device;
        }
        plg->unlock();
        plugin.put("deviceList", deviceList);

        pluginList << plugin;
    }
    reply.put("pluginList", pluginList);
    reply.put("returnValue", true);
    LSError lsError;
    LSErrorInit(&lsError);

    if (msg) {
        if (!LSMessageReply(lsHandle_, msg, reply.stringify().c_str(),
                &lsError)) {
            LOG_ERROR(0, "Message reply error");
            return false;
        }
    } else {
        if (!LSSubscriptionReply(lsHandle_, "getDeviceList",
                reply.stringify().c_str(), &lsError)) {
            LOG_ERROR(0, "Subscription reply error");
            return false;
        }
    }
    return true;
}

bool IndexerService::onPluginGet(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    IndexerService *is = static_cast<IndexerService *>(ctx);
    return is->pluginPutGet(msg, true);
}

bool IndexerService::onPluginPut(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    IndexerService *is = static_cast<IndexerService *>(ctx);
    return is->pluginPutGet(msg, false);
}

bool IndexerService::onPluginListGet(LSHandle *lsHandle, LSMessage *msg,
    void *ctx)
{
    // no schema check needed as we do not expect any objects/properties
    
    // generate response
    auto reply = pbnjson::Object();
    auto pluginList = pbnjson::Array();

    PluginFactory factory;
    const std::list<std::string> &list = factory.plugins();

    for (auto const plg : list) {
        auto plugin = pbnjson::Object();
        plugin.put("uri", plg);
        pluginList << plugin;
    }

    reply.put("pluginList", pluginList);
    reply.put("returnValue", true);

    LSError lsError;
    LSErrorInit(&lsError);

    if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
        LOG_ERROR(0, "Message reply error");
        return false;
    }

    return true;
}

bool IndexerService::onDeviceListGet(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    IndexerService *is = static_cast<IndexerService *>(ctx);
    return is->pushDeviceList(msg);
}

bool IndexerService::onRun(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    IndexerService *is = static_cast<IndexerService *>(ctx);
    return is->detectRunStop(msg, true);
}

bool IndexerService::onStop(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    IndexerService *is = static_cast<IndexerService *>(ctx);
    return is->detectRunStop(msg, false);
}

bool IndexerService::onGetPlaybackUri(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    IndexerService *is = static_cast<IndexerService *>(ctx);

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    pbnjson::JDomParser parser;

    if (!parser.parse(payload, playbackUriGetSchema_)) {
        LOG_ERROR(0, "Invalid %s request: %s", LSMessageGetMethod(msg),
            payload);
        return false;
    }

    auto domTree(parser.getDom());

    // response message
    auto reply = pbnjson::Object();

    RETURN_IF(!domTree.hasKey("uri"), false, "client must specify uri");

    // get the playback uri for the given media item uri
    auto uri = domTree["uri"].asString();
    LOG_DEBUG("Valid %s request for uri: %s", LSMessageGetMethod(msg),
        uri.c_str());

    auto pbUri = is->indexer_->getPlaybackUri(uri);
    reply.put("returnValue", !!pbUri);
    if (pbUri)
        reply.put("playbackUri", pbUri.value());

    LSError lsError;
    LSErrorInit(&lsError);

    if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
        LOG_ERROR(0, "Message reply error");
        return false;
    }

    return true;
}

bool IndexerService::onGetAudioList(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    LOG_DEBUG("call onGetAudioList");
    IndexerService *is = static_cast<IndexerService *>(ctx);
    std::string uri;
    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        LOG_ERROR(0, "Invalid %s request: %s", method.c_str(),
            payload);
        return false;
    }

    auto domTree(parser.getDom());

    if (domTree.hasKey("uri"))
        uri = domTree["uri"].asString();

    bool rv = true;
    MediaDb *mdb = MediaDb::instance();
    // response message
    auto reply = pbnjson::Object();
    if (mdb) {
        pbnjson::JValue resp = pbnjson::Object();
        pbnjson::JValue respArray = pbnjson::Array();
        if (uri.empty()) {
            for (auto const &[_uri, _plg] : is->indexer_->plugins_) {
                for (auto const &[_uri, _dev] : _plg->devices()) {
                    if (_dev->available()) {
                        pbnjson::JValue list = pbnjson::Object();
                        rv &= mdb->getAudioList(_uri, list);
                        list.put("uri", _uri.c_str());
                        respArray.append(list);
                    }
                }
            }
        } else {
            pbnjson::JValue list = pbnjson::Object();
            rv &= mdb->getAudioList(uri, list);
            list.put("uri", uri.c_str());
            respArray.append(list);
        }
        resp.put("audioList", respArray);
        mdb->putRespObject(rv, resp);
        mdb->sendResponse(lsHandle, msg, resp.stringify());
    } else {
        LOG_ERROR(0, "Failed to get instance of Media Db");
        rv = false;
        reply.put("returnValue", rv);
        reply.put("errorCode", -1);
        reply.put("errorText", "Invalid MediaDb Object");

        LSError lsError;
        LSErrorInit(&lsError);

        if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
            LOG_ERROR(0, "Message reply error");
        }
    }

    return rv;
}

bool IndexerService::onGetAudioMetadata(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    LOG_DEBUG("call onGetAudioMetadata");

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        LOG_ERROR(0, "Invalid %s request: %s", LSMessageGetMethod(msg),
            payload);
        return false;
    }

    auto domTree(parser.getDom());
    RETURN_IF(!domTree.hasKey("uri"), false, "client must specify uri");
    // get the playback uri for the given media item uri
    auto uri = domTree["uri"].asString();
    LOG_DEBUG("Valid %s request for uri: %s", LSMessageGetMethod(msg),
        uri.c_str());
    bool rv = false;
    auto mdb = MediaDb::instance();
    auto reply = pbnjson::Object();
    if (mdb) {
        pbnjson::JValue resp = pbnjson::Object();
        rv = mdb->getAudioMetadata(uri, resp);
        mdb->putRespObject(rv, resp);
        mdb->sendResponse(lsHandle, msg, resp.stringify());
    } else {
        LOG_ERROR(0, "Failed to get instance of Media Db");
        reply.put("returnValue", false);
        reply.put("errorCode", -1);
        reply.put("errorText", "Invalid MediaDb Object");

        LSError lsError;
        LSErrorInit(&lsError);

        if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
            LOG_ERROR(0, "Message reply error");
        }
    }

    return rv;

}

bool IndexerService::onGetVideoList(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    LOG_DEBUG("call onGetVideoList");
    IndexerService *is = static_cast<IndexerService *>(ctx);
    std::string uri;
    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        LOG_ERROR(0, "Invalid %s request: %s", LSMessageGetMethod(msg),
            payload);
        return false;
    }

    auto domTree(parser.getDom());

    if (domTree.hasKey("uri"))
        uri = domTree["uri"].asString();
    
    bool rv = true;
    auto mdb = MediaDb::instance();
    auto reply = pbnjson::Object();
    if (mdb) {
        pbnjson::JValue resp = pbnjson::Object();
        pbnjson::JValue respArray = pbnjson::Array();
        if (uri.empty()) {
            for (auto const &[_uri, _plg] : is->indexer_->plugins_) {
                for (auto const &[_uri, _dev] : _plg->devices()) {
                    if (_dev->available()) {
                        pbnjson::JValue list = pbnjson::Object();
                        rv &= mdb->getVideoList(_uri, list);
                        list.put("uri", _uri.c_str());
                        respArray.append(list);
                    }
                }
            }
        } else {
            pbnjson::JValue list = pbnjson::Object();
            rv &= mdb->getVideoList(uri, list);
            list.put("uri", uri.c_str());
            respArray.append(list);
        }
        resp.put("videoList", respArray);
        mdb->putRespObject(rv, resp);
        mdb->sendResponse(lsHandle, msg, resp.stringify());
    } else {
        LOG_ERROR(0, "Failed to get instance of Media Db");
        rv = false;
        reply.put("returnValue", rv);
        reply.put("errorCode", -1);
        reply.put("errorText", "Invalid MediaDb Object");

        LSError lsError;
        LSErrorInit(&lsError);

        if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
            LOG_ERROR(0, "Message reply error");
        }
    }

    return rv;
}

bool IndexerService::onGetVideoMetadata(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    LOG_DEBUG("call onGetVideoMetadata");

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        LOG_ERROR(0, "Invalid %s request: %s", LSMessageGetMethod(msg),
            payload);
        return false;
    }

    auto domTree(parser.getDom());
    RETURN_IF(!domTree.hasKey("uri"), false, "client must specify uri");
    // get the playback uri for the given media item uri
    auto uri = domTree["uri"].asString();
    LOG_DEBUG("Valid %s request for uri: %s", LSMessageGetMethod(msg),
        uri.c_str());
    bool rv = false;
    auto mdb = MediaDb::instance();
    auto reply = pbnjson::Object();
    if (mdb) {
        pbnjson::JValue resp = pbnjson::Object();
        rv = mdb->getVideoMetadata(uri, resp);
        mdb->putRespObject(rv, resp);
        mdb->sendResponse(lsHandle, msg, resp.stringify());
    } else {
        LOG_ERROR(0, "Failed to get instance of Media Db");
        reply.put("returnValue", false);
        reply.put("errorCode", -1);
        reply.put("errorText", "Invalid MediaDb Object");

        LSError lsError;
        LSErrorInit(&lsError);

        if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
            LOG_ERROR(0, "Message reply error");
        }
    }

    return rv;
}

bool IndexerService::onGetImageList(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
   LOG_DEBUG("call onGetImageList");
   IndexerService *is = static_cast<IndexerService *>(ctx);
   std::string uri;
   // parse incoming message
   const char *payload = LSMessageGetPayload(msg);
   std::string method = LSMessageGetMethod(msg);
   pbnjson::JDomParser parser;

   if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
       LOG_ERROR(0, "Invalid %s request: %s", LSMessageGetMethod(msg),
           payload);
       return false;
   }

   auto domTree(parser.getDom());

   if (domTree.hasKey("uri"))
       uri = domTree["uri"].asString();

   bool rv = true;
   auto mdb = MediaDb::instance();
   auto reply = pbnjson::Object();
   if (mdb) {
       pbnjson::JValue resp = pbnjson::Object();
       pbnjson::JValue respArray = pbnjson::Array();
       if (uri.empty()) {
           for (auto const &[_uri, _plg] : is->indexer_->plugins_) {
               for (auto const &[_uri, _dev] : _plg->devices()) {
                   if (_dev->available()) {
                       pbnjson::JValue list = pbnjson::Object();
                       rv &= mdb->getImageList(_uri, list);
                       list.put("uri", _uri.c_str());
                       respArray.append(list);
                   }
               }
           }
       } else {
           pbnjson::JValue list = pbnjson::Object();
           rv &= mdb->getImageList(uri, list);
           list.put("uri", uri.c_str());
           respArray.append(list);
       }
       resp.put("imageList", respArray);
       mdb->putRespObject(rv, resp);
       mdb->sendResponse(lsHandle, msg, resp.stringify());
   } else {
       LOG_ERROR(0, "Failed to get instance of Media Db");
       rv = false;
       reply.put("returnValue", rv);
       reply.put("errorCode", -1);
       reply.put("errorText", "Invalid MediaDb Object");

       LSError lsError;
       LSErrorInit(&lsError);

       if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
           LOG_ERROR(0, "Message reply error");
       }
   }

   return rv;
}

bool IndexerService::onGetImageMetadata(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    //IndexerService *is = static_cast<IndexerService *>(ctx);
    LOG_DEBUG("call onGetImageMetadata");

    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    std::string method = LSMessageGetMethod(msg);
    pbnjson::JDomParser parser;

    if (!parser.parse(payload, pbnjson::JSchema::AllSchema())) {
        LOG_ERROR(0, "Invalid %s request: %s", LSMessageGetMethod(msg),
            payload);
        return false;
    }

    auto domTree(parser.getDom());
    RETURN_IF(!domTree.hasKey("uri"), false, "client must specify uri");
    // get the playback uri for the given media item uri
    auto uri = domTree["uri"].asString();
    LOG_DEBUG("Valid %s request for uri: %s", LSMessageGetMethod(msg),
        uri.c_str());
    bool rv = false;
    auto mdb = MediaDb::instance();
    auto reply = pbnjson::Object();
    if (mdb) {
        pbnjson::JValue resp = pbnjson::Object();
        rv = mdb->getImageMetadata(uri, resp);
        mdb->putRespObject(rv, resp);
        mdb->sendResponse(lsHandle, msg, resp.stringify());
    } else {
        LOG_ERROR(0, "Failed to get instance of Media Db");
        reply.put("returnValue", false);
        reply.put("errorCode", -1);
        reply.put("errorText", "Invalid MediaDb Object");

        LSError lsError;
        LSErrorInit(&lsError);

        if (!LSMessageReply(lsHandle, msg, reply.stringify().c_str(), &lsError)) {
            LOG_ERROR(0, "Message reply error");
        }
    }

    return rv;
}

bool IndexerService::pluginPutGet(LSMessage *msg, bool get)
{
    const char *payload = LSMessageGetPayload(msg);
    pbnjson::JDomParser parser;
    LOG_DEBUG("LSMessageGetMethod : %s", LSMessageGetMethod(msg));
    if (!parser.parse(payload, get ? pluginGetSchema_ : pluginPutSchema_)) {
        LOG_ERROR(0, "Invalid %s request: %s", LSMessageGetMethod(msg),
            payload);
        return false;
    }

    auto domTree(parser.getDom());

    // response message
    auto reply = pbnjson::Object();

    // if no uri is given for getPlugin we activate all plugins
    if (get && !domTree.hasKey("uri")) {
        reply.put("returnValue", indexer_->get(""));
    } else {
        auto uri = domTree["uri"].asString();
        LOG_DEBUG("Valid %s request for uri: %s", LSMessageGetMethod(msg),
            uri.c_str());

        if (get)
            reply.put("returnValue", indexer_->get(uri));
        else
            reply.put("returnValue", indexer_->put(uri));
    }

    LSError lsError;
    LSErrorInit(&lsError);

    if (!LSMessageReply(lsHandle_, msg, reply.stringify().c_str(), &lsError)) {
        LOG_ERROR(0, "Message reply error");
        return false;
    }
    return true;
}

bool IndexerService::detectRunStop(LSMessage *msg, bool run)
{
    // parse incoming message
    const char *payload = LSMessageGetPayload(msg);
    pbnjson::JDomParser parser;
    if (!parser.parse(payload, detectRunStopSchema_)) {
        LOG_ERROR(0, "Invalid %s request: %s", LSMessageGetMethod(msg),
            payload);
        return false;
    }

    auto domTree(parser.getDom());
    if (domTree.hasKey("uri")) {
        auto uri = domTree["uri"].asString();
        LOG_DEBUG("Valid %s request for uri: %s", LSMessageGetMethod(msg),
            uri.c_str());
        indexer_->setDetect(run, uri);
    } else {
        LOG_DEBUG("setDetect Start");
        indexer_->setDetect(run);
    }

    // generate response
    auto reply = pbnjson::Object();
    reply.put("returnValue", true);

    LSError lsError;
    LSErrorInit(&lsError);

    if (!LSMessageReply(lsHandle_, msg, reply.stringify().c_str(), &lsError)) {
        LOG_ERROR(0, "Message reply error");
        return false;
    }
    LOG_DEBUG("detectRunStop Done");
    return true;
}

void IndexerService::checkForDeviceListSubscriber(LSMessage *msg,
    pbnjson::JDomParser &parser)
{
    auto domTree(parser.getDom());
    auto subscribe = domTree["subscribe"].asBool();

    if (!subscribe)
        return;

    LOG_INFO(0, "Adding getDeviceList subscriber '%s'",
        LSMessageGetSenderServiceName(msg));
    LSSubscriptionAdd(lsHandle_, "getDeviceList", msg, NULL);

    auto mdb = MediaDb::instance();
    std::string sn(LSMessageGetSenderServiceName(msg));
    // we still have to strip the -<pid> trailer from the service
    // name
    auto pos = sn.find_last_of("-");
    if (pos != std::string::npos)
        sn.erase(pos);
    mdb->grantAccess(sn);
}
