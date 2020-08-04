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

#ifndef MEDIA_INDEXER_CLIENT_CONNECTOR_H_
#define MEDIA_INDEXER_CLIENT_CONNECTOR_H_

#include <string>
#include "lunaconnector.h"

class Connector {
public:
    Connector(std::string&& serviceName);
    ~Connector();

    // TODO: need to check it we can use this static function
    //static bool onLunaResponse(LSHandle* lsHandle, LSMessage* msg, void* ctx);

    virtual bool handleLunaResponse(LSMessage*) = 0;
    virtual std::string sendMessage(std::string& url, std::string&& request) = 0;
    std::string getServiceName() const;

protected:
    std::unique_ptr<LunaConnector> connector_;
    std::mutex mutex_;
    std::string response_;
private:
    std::string serviceName_;
};

#endif // MEDIA_INDEXER_CLIENT_CONNECTOR_H_
