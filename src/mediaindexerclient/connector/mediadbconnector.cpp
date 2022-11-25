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
const char *MediaDBConnector::dbUrl_ = "luna://com.webos.mediadb/";

MediaDBConnector::MediaDBConnector()
    : Connector(mediaDBClientService_)
{
}

MediaDBConnector::~MediaDBConnector()
{
}

std::string MediaDBConnector::sendMessage(std::string& url, std::string&& request)
{
    LSMessageToken sessionToken;
    bool async = false;

    if (!connector_->sendMessage(url.c_str(), request.c_str(),
                                 MediaDBConnector::onLunaResponse, this,
                                 async, &sessionToken)) {
        std::cout << "sendMessage ERROR!" << std::endl;
        return std::string();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    return response_;
}

std::string MediaDBConnector::sendSearchMessage(std::string&& request)
{
    LSMessageToken sessionToken;
    bool async = false;

    std::string url = ((dbUrl_ != nullptr) ? dbUrl_ : "") + std::string("search");
    std::cout << "Url : " << url << std::endl;
    std::cout << "request : " << request << std::endl;

    if (!connector_->sendMessage(url.c_str(), request.c_str(),
                                 MediaDBConnector::onLunaResponse, this,
                                 async, &sessionToken)) {
        std::cout << "sendMessage ERROR!" << std::endl;
        return std::string();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    return response_;
}

std::string MediaDBConnector::sendDelMessage(std::string&& request)
{
    LSMessageToken sessionToken;
    bool async = false;

    std::string url = ((dbUrl_ != nullptr) ? dbUrl_ : "") + std::string("del");
    std::cout << "Url : " << url << std::endl;
    std::cout << "request : " << request << std::endl;

    if (!connector_->sendMessage(url.c_str(), request.c_str(),
                                 MediaDBConnector::onLunaResponse, this,
                                 async, &sessionToken)) {
        std::cout << "sendMessage ERROR!" << std::endl;
        return std::string();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    return response_;
}


std::string MediaDBConnector::getDBUrl() const
{
    return dbUrl_;
}

bool MediaDBConnector::onLunaResponse(LSHandle* lsHandle, LSMessage* msg, void* ctx)
{
    MediaDBConnector *conn = static_cast<MediaDBConnector*>(ctx);
    return conn->handleLunaResponse(msg);
}

bool MediaDBConnector::handleLunaResponse(LSMessage* msg)
{
    pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());
    const char *payload = LSMessageGetPayload(msg);

    if (!parser.parse(payload)) {
        std::cout << "Invalid JSON message: " << payload << std::endl;
        return false;
    }

    pbnjson::JValue domTree(parser.getDom());

    std::lock_guard<std::mutex> lock(mutex_);
    response_ = domTree.stringify();

    JSchemaInfo schemaInfo;
    jschema_info_init(&schemaInfo, jschema_all(), NULL, NULL);
    jvalue_ref object = jdom_parse(j_cstr_to_buffer(payload), DOMOPT_NOOPT, &schemaInfo);
    pretty_print(object, 0, 4);

    return true;
}
