// Copyright (c) 2020 LG Electronics, Inc.
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

#include "mediadbconnector.h"

const char *MediaDBConnector::mediaDBClientService_ = "com.webos.service.mediaindexer.client.db";
const char *MediaDBConnector::dbUrl_ = "luna://com.webos.service.db/";

MediaDBConnector::MediaDBConnector()
    : Connector(mediaDBClientService_)
{
    std::cout << "I'm MediaDBConnector" << std::endl;
}

MediaDBConnector::~MediaDBConnector()
{
    std::cout << "I'm ~MediaDBConnector" << std::endl;
}

std::string MediaDBConnector::sendMessage(std::string& url, std::string&& request)
{
    std::cout << "I'm MediaDBConnector::sendMessage" << std::endl;
    LSMessageToken sessionToken;
    bool async = false;

    if (!connector_->sendMessage(url.c_str(), request.c_str(), 
                                 MediaDBConnector::onLunaResponse, this,
                                 async, &sessionToken)) {
        std::cout << "sendMessage ERROR!" << std::endl;
        return std::string();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "Return value : " << response_ << std::endl;
    std::cout << "I'm MediaDBConnector::sendMessage END!!!!!!!" << std::endl;
    return response_;
}

std::string MediaDBConnector::sendSearchMessage(std::string&& request)
{
    std::cout << "I'm MediaDBConnector::sendSearchMessage" << std::endl;
    std::cout << "request param : " << request << std::endl;
    LSMessageToken sessionToken;
    bool async = false;

    std::string url = dbUrl_ + std::string("search");
    std::cout << "Url : " << url << std::endl;

    if (!connector_->sendMessage(url.c_str(), request.c_str(), 
                                 MediaDBConnector::onLunaResponse, this,
                                 async, &sessionToken)) {
        std::cout << "sendMessage ERROR!" << std::endl;
        return std::string();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "Return value : " << response_ << std::endl;
    std::cout << "I'm MediaDBConnector::sendSearchMessage END!!!!!!!" << std::endl;
    return response_;
}

std::string MediaDBConnector::getDBUrl() const
{
    return dbUrl_;
}

bool MediaDBConnector::onLunaResponse(LSHandle* lsHandle, LSMessage* msg, void* ctx)
{
    MediaDBConnector *conn = static_cast<MediaDBConnector*>(ctx);
    std::cout << "I'm MediaDBConnector::onLunaResponse" << std::endl;
    return conn->handleLunaResponse(msg);
}

bool MediaDBConnector::handleLunaResponse(LSMessage* msg)
{
    std::cout << "I'm MediaDBConnector::handleLunaResponse" << std::endl;
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
    response_ = domTree.stringify();
    return true;
}
