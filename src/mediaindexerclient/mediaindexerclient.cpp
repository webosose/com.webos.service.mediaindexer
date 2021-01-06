// Copyright (c) 2020-2021 LG Electronics, Inc.
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
#include <list>
#include <functional>

#include <gio/gio.h>

MediaIndexerClient::MediaIndexerClient(MediaIndexerCallback cb, void* userData)
    : callback_(cb),
      userData_(userData)
{
    mediaDBConnector_ = std::unique_ptr<MediaDBConnector>(new MediaDBConnector);

    if (!mediaDBConnector_)
        std::cout << "Failed to create mediaDBConnector object" << std::endl;
    //std::cout << "mediaDBConnector service name : " << mediaDBConnector_->getServiceName() << std::endl;

    indexerConnector_ = std::unique_ptr<IndexerConnector>(new IndexerConnector);

    if (!indexerConnector_)
        std::cout << "Failed to create indexerConnector object" << std::endl;
    //std::cout << "indexerConnector service name : " << indexerConnector_->getServiceName() << std::endl;

}

MediaIndexerClient::~MediaIndexerClient()
{
    mediaDBConnector_.reset();
    indexerConnector_.reset();
}

void MediaIndexerClient::initialize() const
{
    //let's get the DB permission
    getMediaDBPermission();
}

std::string MediaIndexerClient::getAudioList(const std::string& uri) const
{
    if (!mediaDBConnector_) {
        std::cout << "mediaDBConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "[START] getAudioList" << std::endl;
    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetAudioListAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "[END] getAudioList" << std::endl;
    return ret;
}

std::string MediaIndexerClient::getVideoList(const std::string& uri) const
{
    if (!mediaDBConnector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "[START] getVideoList" << std::endl;
    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetVideoListAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "[END] getVideoList" << std::endl;
    return ret;
}

std::string MediaIndexerClient::getImageList(const std::string& uri) const
{
    if (!mediaDBConnector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "[START] getImageList" << std::endl;
    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetImageListAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "[END] getImageList" << std::endl;
    return ret;
}

std::string MediaIndexerClient::getAudioMetaData(const std::string& uri) const
{
    if (!mediaDBConnector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    if (uri.empty()) {
        std::cout << "Uri is NULL!. Input uri" << std::endl;
        return std::string();
    }

    std::cout << "[START] getAudioMetaData" << std::endl;
    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetAudioMetaDataAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "[END] getAudioMetaData" << std::endl;
    return ret;
}

std::string MediaIndexerClient::getVideoMetaData(const std::string& uri) const
{
    if (!mediaDBConnector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    if (uri.empty()) {
        std::cout << "Uri is NULL!. Input uri" << std::endl;
        return std::string();
    }

    std::cout << "[START] getVideoMetaData" << std::endl;
    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetVideoMetaDataAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "[END] getVideoMetaData" << std::endl;
    return ret;
}

std::string MediaIndexerClient::getImageMetaData(const std::string& uri) const
{
    if (!mediaDBConnector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    if (uri.empty()) {
        std::cout << "Uri is NULL!. Input uri" << std::endl;
        return std::string();
    }

    std::cout << "[START] getImageMetaData" << std::endl;
    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetImageMetaDataAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "[END] getImageMetaData" << std::endl;
    return ret;
}

std::string MediaIndexerClient::requestDelete(const std::string& uri) const
{
    if (!mediaDBConnector_) {
        std::cout << "mediaDBConnector is NULL!" << std::endl;
        return std::string();
    }

    if (uri.empty()) {
        std::cout << "Uri is NULL!. Input uri" << std::endl;
        return std::string();
    }

    std::cout << "[START] requestDelete" << std::endl;
    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::RequestDelete, uri);
    std::string ret = mediaDBConnector_->sendDelMessage(request.stringify());
    std::cout << "[END] requestDelete" << std::endl;
    return ret;
}

std::string MediaIndexerClient::requestMediaScan(const std::string& path) const
{
    if (!mediaDBConnector_ || !indexerConnector_)
        return std::string();;

    if (path.empty()) {
        std::cout << "path is NULL!. Input path" << std::endl;
        return std::string();
    }

    std::cout << "[START] requestMediaScan" << std::endl;

    std::string url = indexerConnector_->getIndexerUrl();
    url.append(std::string("requestMediaScan"));
    auto request = pbnjson::Object();
    request.put("path", path);
    std::cout << "requestMediaScan url : " << url << std::endl;
    std::cout << "requestMediaScan request : " << request.stringify() << std::endl;

    std::string ret = indexerConnector_->sendMessage(url, request.stringify());
    std::cout << "[END] requestMediaScan" << std::endl;
    return ret;
}

// TODO: remove duplicated append for each requested function.
pbnjson::JValue MediaIndexerClient::generateLunaPayload(MediaIndexerClientAPI api,
                                                        const std::string& uri) const
{
    auto request = pbnjson::Object();
    switch(api) {
        case MediaIndexerClientAPI::GetAudioListAPI: {
            auto selectArray = pbnjson::Array();
            selectArray.append(std::string("uri"));
            selectArray.append(std::string("type"));
            selectArray.append(std::string("last_modified_date"));
            selectArray.append(std::string("file_size"));
            selectArray.append(std::string("file_path"));
            selectArray.append(std::string("title"));
            selectArray.append(std::string("duration"));
            selectArray.append(std::string("thumbnail"));

            auto query = pbnjson::Object();
            auto wheres = pbnjson::Array();
            if (uri.empty()) {
                prepareWhere("dirty", false, true, wheres);
            } else {
                prepareWhere("dirty", false, true, wheres);
                prepareWhere("uri", uri, false, wheres);
            }
            query = prepareQuery(selectArray, audioKind_, wheres);
            request.put("query", query);
            break;
        }
        case MediaIndexerClientAPI::GetVideoListAPI: {
            auto selectArray = pbnjson::Array();
            selectArray.append(std::string("uri"));
            selectArray.append(std::string("type"));
            selectArray.append(std::string("last_modified_date"));
            selectArray.append(std::string("file_size"));
            selectArray.append(std::string("file_path"));
            selectArray.append(std::string("duration"));
            selectArray.append(std::string("title"));
            selectArray.append(std::string("thumbnail"));

            auto query = pbnjson::Object();
            auto wheres = pbnjson::Array();
            if (uri.empty()) {
                prepareWhere("dirty", false, true, wheres);
            } else {
                prepareWhere("dirty", false, true, wheres);
                prepareWhere("uri", uri, false, wheres);
            }
            query = prepareQuery(selectArray, videoKind_, wheres);
            request.put("query", query);
            break;
        }
        case MediaIndexerClientAPI::GetImageListAPI: {
            auto selectArray = pbnjson::Array();
            selectArray.append(std::string("uri"));
            selectArray.append(std::string("type"));
            selectArray.append(std::string("last_modified_date"));
            selectArray.append(std::string("file_size"));
            selectArray.append(std::string("file_path"));
            selectArray.append(std::string("title"));
            selectArray.append(std::string("width"));
            selectArray.append(std::string("height"));

            auto query = pbnjson::Object();
            auto wheres = pbnjson::Array();
            if (uri.empty()) {
                prepareWhere("dirty", false, true, wheres);
            } else {
                prepareWhere("dirty", false, true, wheres);
                prepareWhere("uri", uri, false, wheres);
            }
            query = prepareQuery(selectArray, imageKind_, wheres);
            request.put("query", query);
            break;
        }
        case MediaIndexerClientAPI::GetAudioMetaDataAPI: {
            auto selectArray = pbnjson::Array();
            selectArray.append(std::string("uri"));
            selectArray.append(std::string("mime"));
            selectArray.append(std::string("type"));
            selectArray.append(std::string("date_of_creation"));
            selectArray.append(std::string("last_modified_date"));
            selectArray.append(std::string("file_size"));
            selectArray.append(std::string("file_path"));
            selectArray.append(std::string("title"));
            selectArray.append(std::string("genre"));
            selectArray.append(std::string("album"));
            selectArray.append(std::string("artist"));
            selectArray.append(std::string("album_artist"));
            selectArray.append(std::string("track"));
            selectArray.append(std::string("total_tracks"));
            selectArray.append(std::string("duration"));
            selectArray.append(std::string("thumbnail"));
            selectArray.append(std::string("sample_rate"));
            selectArray.append(std::string("bit_per_sample"));
            selectArray.append(std::string("bit_rate"));
            selectArray.append(std::string("channels"));
            selectArray.append(std::string("lyric"));

            auto wheres = pbnjson::Array();
            prepareWhere("uri", uri, false, wheres);
            prepareWhere("dirty", false, true, wheres);
            auto query = prepareQuery(selectArray, audioKind_, wheres);
            request.put("query", query);
            break;
        }
        case MediaIndexerClientAPI::GetVideoMetaDataAPI: {
            auto selectArray = pbnjson::Array();
            selectArray.append(std::string("uri"));
            selectArray.append(std::string("title"));
            selectArray.append(std::string("mime"));
            selectArray.append(std::string("type"));
            selectArray.append(std::string("date_of_creation"));
            selectArray.append(std::string("last_modified_date"));
            selectArray.append(std::string("file_size"));
            selectArray.append(std::string("file_path"));
            selectArray.append(std::string("duration"));
            selectArray.append(std::string("width"));
            selectArray.append(std::string("height"));
            selectArray.append(std::string("thumbnail"));
            selectArray.append(std::string("frame_rate"));

            auto wheres = pbnjson::Array();
            prepareWhere("uri", uri, false, wheres);
            prepareWhere("dirty", false, true, wheres);
            auto query = prepareQuery(selectArray, videoKind_, wheres);
            request.put("query", query);
            break;
        }
        case MediaIndexerClientAPI::GetImageMetaDataAPI: {
            auto selectArray = pbnjson::Array();
            selectArray.append(std::string("uri"));
            selectArray.append(std::string("mime"));
            selectArray.append(std::string("title"));
            selectArray.append(std::string("type"));
            selectArray.append(std::string("date_of_creation"));
            selectArray.append(std::string("last_modified_date"));
            selectArray.append(std::string("file_size"));
            selectArray.append(std::string("file_path"));
            selectArray.append(std::string("width"));
            selectArray.append(std::string("height"));
            selectArray.append(std::string("geo_location_city"));
            selectArray.append(std::string("geo_location_country"));
            selectArray.append(std::string("geo_location_latitude"));
            selectArray.append(std::string("geo_location_longitude"));

            auto wheres = pbnjson::Array();
            prepareWhere("uri", uri, false, wheres);
            prepareWhere("dirty", false, true, wheres);
            auto query = prepareQuery(selectArray, imageKind_, wheres);
            request.put("query", query);
            break;
        }
        case MediaIndexerClientAPI::RequestDelete: {
            auto where = pbnjson::Array();
            prepareWhere("uri", uri, false, where);
            std::string kindId = getKindID(uri);
            auto query = prepareQuery(kindId, where);
            request.put("query", query);
            break;
        }
        default:
            break;
    }

    return request;
}

std::string MediaIndexerClient::getKindID(const std::string &uri) const
{
    std::string kindId;
    gboolean uncertain;
    bool mimeTypeSupported = false;
    gchar *contentType = g_content_type_guess(uri.c_str(), nullptr, 0, &uncertain);

    if (!contentType) {
        std::cout << "contentType is NULL for '" << uri << "'" << std::endl;
        return std::string();
    }

    kindId = typeFromMime(contentType);
    if (!kindId.length() > 0) {
        /* get the file extension for ts or ps */
        std::string mimeType;
        std::string ext = uri.substr(uri.find_last_of('.') + 1);
        if (!ext.compare("ts")) {
            std::cout << "Found ts extension!" << std::endl;
            mimeType = std::string("video/MP2T");
        }
        else if (!ext.compare("ps")) {
            std::cout << "Found ps extension!" << std::endl;
            mimeType = std::string("video/MP2P");
        }
        else {
            std::cout << "Not supported type" << std::endl;
            return std::string();
        }
        kindId = typeFromMime(mimeType);
        return kindId;
    }
    g_free(contentType);
    return kindId;
}

std::string MediaIndexerClient::typeFromMime(const std::string &mime) const
{
    std::list<std::string> typelist = {"audio", "video", "image"};
    for (auto type : typelist) {
        if (!mime.compare(0, type.size(), type))
            return type;
    }
    return "";
}

bool MediaIndexerClient::prepareWhere(const std::string &key,
                                                 const std::string &value,
                                                 bool precise,
                                                 pbnjson::JValue &whereClause) const
{
    auto cond = pbnjson::Object();
    cond.put("prop", key);
    cond.put("op", precise ? "=" : "%");
    cond.put("val", value);
    whereClause << cond;
    return true;
}

bool MediaIndexerClient::prepareWhere(const std::string &key,
                                                 bool value,
                                                 bool precise,
                                                 pbnjson::JValue &whereClause) const
{
    auto cond = pbnjson::Object();
    cond.put("prop", key);
    cond.put("op", precise ? "=" : "%");
    cond.put("val", value);
    whereClause << cond;
    return true;
}

pbnjson::JValue MediaIndexerClient::prepareQuery(   const std::string& kindId,
                                                 pbnjson::JValue where) const
{
    auto query = pbnjson::Object();
    query.put("from", kindId);
    query.put("where", where);
    return query;
}

pbnjson::JValue MediaIndexerClient::prepareQuery(pbnjson::JValue selectArray,
                                                 const std::string& kindId,
                                                 pbnjson::JValue where) const
{
    auto query = pbnjson::Object();
    query.put("select", selectArray);
    query.put("from", kindId);
    query.put("where", where);
    return query;
}

std::string MediaIndexerClient::getDeviceList() const
{
    if (!indexerConnector_) {
        std::cout << "indexerConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "[START] getDeviceList" << std::endl;

    std::string url = indexerConnector_->getIndexerUrl();
    url.append(std::string("getDeviceList"));
    auto request = pbnjson::Object();
    request.put("subscribe", true);
    std::cout << "getDeviceList url : " << url << std::endl;
    std::cout << "getDeviceList request : " << request.stringify() << std::endl;

    std::string ret = indexerConnector_->sendMessage(url, request.stringify());
    std::cout << "[END] getDeviceList" << std::endl;
    return ret;
}

void MediaIndexerClient::getMediaDBPermission() const
{
    if (!mediaDBConnector_ || !indexerConnector_)
        return;

    std::cout << "[START] getMediaDBPermission" << std::endl;

    std::string url = indexerConnector_->getIndexerUrl();
    url.append(std::string("getMediaDbPermission"));
    auto request = pbnjson::Object();
    request.put("serviceName", mediaDBConnector_->getServiceName());
    std::cout << "getMediaDBPermission url : " << url << std::endl;
    std::cout << "getMediaDBPermission request : " << request.stringify() << std::endl;

    std::string ret = indexerConnector_->sendMessage(url, request.stringify());
    std::cout << "[END] getMediaDBPermission \n" << std::endl;
}
