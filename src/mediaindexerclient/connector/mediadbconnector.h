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

#include "connector.h"

#ifndef MEDIA_INDEXER_CLIENT_MEDIADB_CONNECTOR_H_
#define MEDIA_INDEXER_CLIENT_MEDIADB_CONNECTOR_H_


class MediaDBConnector : public Connector {
public:
    MediaDBConnector();
    ~MediaDBConnector();

    std::string sendMessage(std::string& url, std::string&& request) override;
    std::string sendSearchMessage(std::string&& request);
    std::string sendDelMessage(std::string&& request);
    std::string getDBUrl() const;
private:
    // Luna response callback and handler.
    static bool onLunaResponse(LSHandle* lsHandle, LSMessage* msg, void* ctx);
    virtual bool handleLunaResponse(LSMessage* msg) override;

    static const char *mediaDBClientService_;
    static const char *dbUrl_;
};

#endif // MEDIA_INDEXER_CLIENT_MEDIADB_CONNECTOR_H_
