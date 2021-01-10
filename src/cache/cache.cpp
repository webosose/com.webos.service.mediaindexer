// Copyright (c) 2019-2021 LG Electronics, Inc.
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

#include "cache.h"
#include <fstream>
#include <filesystem>
#include <cinttypes>

Cache::Cache(const std::string& path)
    : cachePath_(path)
{
    LOG_DEBUG("Cache ctor! path : %s", cachePath_.c_str());
}

Cache::~Cache()
{
    LOG_DEBUG("Cache dtor! path : %s", cachePath_.c_str());
    cacheMap_.clear();
}

void Cache::insertItem(const std::string& uri, const unsigned long& hash)
{
    cacheMap_.emplace(uri, hash);
}

int Cache::size() const
{
    return cacheMap_.size();
}

const std::string& Cache::getPath() const
{
    return cachePath_;
}

bool Cache::setPath(const std::string& path)
{
    if (path.empty()) {
        LOG_WARNING(0, "given cache path is empty!");
        return false;
    }

    cachePath_ = path;
    return true;
}

bool Cache::generateCacheFile()
{
    // remove cache file first
    std::string path = getPath();
    std::filesystem::remove(path);

    std::ofstream outputFile(path);
    if (!outputFile.is_open()) {
        LOG_ERROR(0, "cache file generation fail! need to check '%s'", path.c_str());
        return false;
    }

    //JSonComposer composer;
    //auto cache = Composer.tojson(path);
    auto cache = pbnjson::Object();
    auto uri_array = pbnjson::Array();
    auto hash_array = pbnjson::Array();
    for (auto item : cacheMap_) {
        uri_array.append(item.first);
        hash_array.append(std::to_string(item.second));
    }
    cache.put("uri", uri_array);
    cache.put("hash", hash_array);

    outputFile << pbnjson::JGenerator::serialize(cache, pbnjson::JSchemaFragment("{}"));
    outputFile.close();
    cacheMap_.clear();

    return true;
}

bool Cache::readCache()
{
    std::string path = getPath();
    // get the JDOM tree from given path
    auto root = pbnjson::JDomParser::fromFile(path.c_str());
    if (!root.isObject()) {
        LOG_ERROR(0, "cache file parsing error! need to check '%s'", path.c_str());
        return false;
    }

    if (!root.hasKey("uri") || !root.hasKey("hash")) {
        LOG_WARNING(0, "can't find 'uri' and 'hash' field!");
        return false;
    }

    auto uriList = root["uri"];
    auto hashList = root["hash"];
    int uri_count = uriList.arraySize();
    int hash_count = hashList.arraySize();

    if (uri_count != hash_count) {
        LOG_WARNING(0, "count mismatch between 'uriList' and 'hashList'");
        return false;
    }

    for (int idx = 0; idx < uri_count; idx++) {
        auto uri = uriList[idx].asString();
        auto hash = std::stoul(hashList[idx].asString());
        cacheMap_.emplace(uri, hash);
    }

    // remove cache file
    std::filesystem::remove(getPath());
    return true;
}

bool Cache::isExist(const std::string& uri, const unsigned long& hash)
{
    auto ret = cacheMap_.find(uri);
    if (ret != cacheMap_.end() && ret->second == hash) {
        return true;
    } else {
        cacheMap_.erase(uri);
        return false;
    }
}

void Cache::resetCache()
{
    std::filesystem::remove(getPath());
    cacheMap_.clear();
}

void Cache::clear()
{
    cacheMap_.clear();
}

void Cache::printCache() const
{
    LOG_DEBUG("--------------Cached Items--------------");
    for (const auto &cache : cacheMap_)
        LOG_DEBUG("uri : '%s', hash : '%"PRIu64"'", cache.first.c_str(), cache.second);
    LOG_DEBUG("-------------------------  -------------");
}
