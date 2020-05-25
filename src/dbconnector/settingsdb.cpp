// Copyright (c) 2019 LG Electronics, Inc.
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

#include "settingsdb.h"
#include "mediaindexer.h"

std::unique_ptr<SettingsDb> SettingsDb::instance_;

SettingsDb *SettingsDb::instance()
{
    if (!instance_.get()) {
        instance_.reset(new SettingsDb);
        instance_->ensureKind();
    }
    return instance_.get();
}

SettingsDb::~SettingsDb()
{
    // nothing to be done here
}

void SettingsDb::applySettings(const std::string &uri)
{
    LOG_INFO(0, "Search in settings database for '%s'", uri.c_str());
    find(uri, true);
}

void SettingsDb::setEnable(const std::string &uri, bool enable)
{
    auto props = pbnjson::Object();
    props.put("uri", uri);
    props.put("enabled", enable);

    mergePut(uri, true, props);
}

bool SettingsDb::handleLunaResponse(LSMessage *msg)
{
    struct SessionData sd;
    if (!sessionDataFromToken(LSMessageGetResponseToken(msg), &sd))
        return false;

    auto method = sd.method;
    LOG_INFO(0, "Received response com.webos.service.db for: '%s'",
        method.c_str());

    if (method != std::string("find"))
        return true;

    // we do not need to check, the service implementation should do that
    pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());
    const char *payload = LSMessageGetPayload(msg);

    if (!parser.parse(payload)) {
        LOG_ERROR(0, "Invalid JSON message: %s", payload);
        return false;
    }

    pbnjson::JValue domTree(parser.getDom());

    if (!domTree.hasKey("results"))
        return false;

    auto matches = domTree["results"];

    // sanity check
    if (!matches.isArray())
        return true;

    for (ssize_t i = 0; i < matches.arraySize(); ++i) {
        auto match = matches[i];

        auto uri = match["uri"].asString();
        auto enabled = match["enabled"].asBool();

        MediaIndexer *mi = MediaIndexer::instance();
        mi->get(uri);
        mi->setDetect(enabled, uri);
    }

    return true;
}

SettingsDb::SettingsDb() :
    DbConnector("com.webos.service.mediaindexer.settings:1")
{
    auto index = pbnjson::Object();
    index.put("name", "uri");

    auto props = pbnjson::Array();
    auto prop = pbnjson::Object();
    prop.put("name", "uri");
    props << prop;

    index.put("props", props);

    kindIndexes_ << index;
}
