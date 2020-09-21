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

#include "indexerconnector.h"

const char *IndexerConnector::indexerUrl_ = "luna://com.webos.service.mediaindexer/";
const char *IndexerConnector::indexerClientService_ = "com.webos.service.mediaindexer.client";
IndexerConnector::IndexerConnector()
    : Connector(indexerClientService_)
{
}

IndexerConnector::~IndexerConnector()
{
}

std::string IndexerConnector::sendMessage(std::string& url, std::string&& request)
{
    LSMessageToken sessionToken;
    bool async = false;

    if (!connector_->sendMessage(url.c_str(), request.c_str(),
                                 IndexerConnector::onLunaResponse, this,
                                 async, &sessionToken)) {
        std::cout << "sendMessage ERROR!" << std::endl;
        return std::string();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    return response_;
}

std::string IndexerConnector::getIndexerUrl() const
{
    return indexerUrl_;
}

bool IndexerConnector::onLunaResponse(LSHandle* lsHandle, LSMessage* msg, void* ctx)
{
    IndexerConnector *conn = static_cast<IndexerConnector*>(ctx);
    return conn->handleLunaResponse(msg);
}

bool IndexerConnector::handleLunaResponse(LSMessage* msg)
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
