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
#include "GenerateUniqueID.h"

Connector::Connector(std::string&& serviceName)
    : serviceName_(serviceName)
{
    serviceName_ += mediaindexerclient::GenerateUniqueID()();
    std::cout << "I'm Connector! serviceName : " << serviceName_ << std::endl;
    connector_ = std::unique_ptr<LunaConnector>(new LunaConnector(serviceName_, true));
}

Connector::~Connector()
{ 
    connector_.reset();
}

std::string Connector::getServiceName() const
{
    return serviceName_;
}

/*
bool Connector::onLunaResponse(LSHandle* lsHandle, LSMessage* msg, void* ctx)
{
    Connector *conn = static_cast<Connector*>(ctx);
    std::cout << "I'm onLunaResponse!" << std::endl;
    return conn->handleLunaResponse(msg);
}
*/
