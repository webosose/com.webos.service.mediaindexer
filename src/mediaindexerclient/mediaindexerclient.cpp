// Copyright (c) 2018-2020 LG Electronics, Inc.
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

#include "mediaindexerclient.h"
#include <iostream>

const char* MediaIndexerClient::dbUrl_ = "luna://com.webos.service.db/";

MediaIndexerClient::MediaIndexerClient(MediaIndexerCallback cb, void* userData)
    : callback_(cb),
      userData_(userData)
{
    std::cout << std::string("MediaIndexerClient ctor!") << std::endl;

    connector_ = std::unique_ptr<LunaConnector>(new LunaConnector(std::string("com.webos.service.mediaindexer.media"), true));

    if (!connector_)
        std::cout << "Failed to create lunaconnector object" << std::endl;
}

std::string MediaIndexerClient::getAudioList(const std::string& uri)
{
    if (!connector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "I'm getAudioList" << std::endl;
    std::cout << "thread id[" << std::this_thread::get_id() << "]" << std::endl;

    auto selectArray = pbnjson::Array();
    selectArray.append(std::string("uri"));
    selectArray.append(std::string("mime"));
    selectArray.append(std::string("title"));
    selectArray.append(std::string("genre"));
    selectArray.append(std::string("album"));
    selectArray.append(std::string("artist"));
    selectArray.append(std::string("album_artist"));
    selectArray.append(std::string("track"));
    selectArray.append(std::string("total_tracks"));
    selectArray.append(std::string("duration"));
    selectArray.append(std::string("thumbnail"));
    selectArray.append(std::string("file_size"));

    pbnjson::JValue resp = pbnjson::Object();
    LSMessageToken sessionToken;
    bool async = false;
    bool precise = false;
    std::string val = "storage";
    std::string url = dbUrl_ + std::string("search");

    auto query = pbnjson::Object();
    query.put("select", selectArray);
    query.put("from", "com.webos.service.mediaindexer.audio:1");
    auto where = pbnjson::Array();
    auto cond = pbnjson::Object();
    cond.put("prop", "uri");
    cond.put("op", precise ? "=" : "%");
    cond.put("val", val);
    where << cond;
    query.put("where", where);

    auto request = pbnjson::Object();
    request.put("query", query);
    
    if (!connector_->sendMessage(url.c_str(), request.stringify().c_str(), MediaIndexerClient::onLunaResponse, this, async, &sessionToken)) {
        std::cout << "sendMessage ERROR!" << std::endl;
        return std::string();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "Return value_ in getAudioList : " << returnValue_ << std::endl;
    std::cout << "I'm getAudioList END!!!!!!!!!!!!" << std::endl;
    return returnValue_;
}

std::string MediaIndexerClient::getVideoList(const std::string& uri)
{
    if (!connector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "I'm getVideoList" << std::endl;
    if (callback_) {
        std::cout << "thread id[" << std::this_thread::get_id() << "]" << std::endl;
        callback_(NotifyGetVideoList, nullptr, nullptr);
    }
    return std::string("I'm getVideoList");
}

std::string MediaIndexerClient::getImageList(const std::string& uri)
{
    if (!connector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "I'm getImageList" << std::endl;
    return std::string("I'm getImageList");
}

std::string MediaIndexerClient::getAudioMetaData(const std::string& uri)
{
    if (!connector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "I'm getAudioMetaData" << std::endl;
    return std::string("I'm getAudioMetaData");
}

std::string MediaIndexerClient::getVideoMetaData(const std::string& uri)
{
    if (!connector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "I'm getVideoMetaData" << std::endl;
    return std::string("I'm getVideoMetaData");
}

std::string MediaIndexerClient::getImageMetaData(const std::string& uri)
{
    if (!connector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "I'm getImageMetaData" << std::endl;
    return std::string("I'm getImageMetaData");
}

bool MediaIndexerClient::onLunaResponse(LSHandle *lsHandle, LSMessage *msg, void *ctx)
{
    MediaIndexerClient *client = static_cast<MediaIndexerClient*>(ctx);
    std::cout << "I'm onLunaResponse" << std::endl;
    return client->handleLunaResponse(msg);
}

bool MediaIndexerClient::handleLunaResponse(LSMessage *msg)
{
    std::cout << "I'm handleLunaResponse" << std::endl;
    std::cout << "thread id[" << std::this_thread::get_id() << "]" << std::endl;

    pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());
    const char *payload = LSMessageGetPayload(msg);

    std::cout << "payload : " << payload << std::endl;

    if (!parser.parse(payload)) {
        std::cout << "Invalid JSON message: " << payload << std::endl;
        return false;
    }

    pbnjson::JValue domTree(parser.getDom());
    std::cout << "Message : " << domTree.stringify() << std::endl;

    std::lock_guard<std::mutex> lock(mutex_);
    returnValue_ = domTree.stringify();
    
    return true;
}

bool MediaIndexerClient::generateLunaPayload(MediaIndexerClientAPI api, const std::string& uri)
{
    switch(api) {
        case GetAudioListAPI: {
            break;
        }
        case GetVideoListAPI: {
            break;
        }
        case GetImageListAPI: {
            break;
        }
        case GetAudioMetaDataAPI: {
            break;
        }
        case GetVideoMetaDataAPI: {
            break;
        }
        case GetImageMetaDataAPI: {
            break;
        default:
            break;
        }
    }

    return true;
}