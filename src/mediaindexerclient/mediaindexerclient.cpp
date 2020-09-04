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

#include "mediaindexerclient.h"
#include <iostream>
#include <list>
#include <functional>

#include <gio/gio.h>

MediaIndexerClient::MediaIndexerClient(MediaIndexerCallback cb, void* userData)
    : callback_(cb),
      userData_(userData)
{
    std::cout << std::string("MediaIndexerClient ctor!") << std::endl;

    mediaDBConnector_ = std::unique_ptr<MediaDBConnector>(new MediaDBConnector);

    if (!mediaDBConnector_)
        std::cout << "Failed to create mediaDBConnector object" << std::endl;
    std::cout << "mediaDBConnector service name : " << mediaDBConnector_->getServiceName() << std::endl;

    indexerConnector_ = std::unique_ptr<IndexerConnector>(new IndexerConnector);

    if (!indexerConnector_)
        std::cout << "Failed to create indexerConnector object" << std::endl;
    std::cout << "indexerConnector service name : " << indexerConnector_->getServiceName() << std::endl;

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

    std::cout << "I'm getAudioList" << std::endl;
    std::cout << "thread id[" << std::this_thread::get_id() << "]" << std::endl;

    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetAudioListAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "Return value_ in getAudioList : " << ret << std::endl;
    std::cout << "I'm getAudioList END!!!!!!!!!!!!" << std::endl;
    return ret;
}

std::string MediaIndexerClient::getVideoList(const std::string& uri) const
{
    if (!mediaDBConnector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "I'm getVideoList" << std::endl;
    std::cout << "thread id[" << std::this_thread::get_id() << "]" << std::endl;

    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetVideoListAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "Return value_ in getVideoList : " << ret << std::endl;
    std::cout << "I'm getVideoList END!!!!!!!!!!!!" << std::endl;
    return ret;
}

std::string MediaIndexerClient::getImageList(const std::string& uri) const
{
    if (!mediaDBConnector_) {
        std::cout << "LunaConnector is NULL!" << std::endl;
        return std::string();
    }

    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetImageListAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "Return value_ in getImageList : " << ret << std::endl;
    std::cout << "I'm getImageList END!!!!!!!!!!!!" << std::endl;
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

    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetAudioMetaDataAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "Return value_ in getAudioMetaData : " << ret << std::endl;
    std::cout << "I'm getAudioMetaData END!!!!!!!!!!!!" << std::endl;
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

    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetVideoMetaDataAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "Return value_ in getVideoMetaData : " << ret << std::endl;
    std::cout << "I'm getVideoMetaData END!!!!!!!!!!!!" << std::endl;
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

    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::GetImageMetaDataAPI, uri);
    std::string ret = mediaDBConnector_->sendSearchMessage(request.stringify());
    std::cout << "Return value_ in getImageMetaData : " << ret << std::endl;
    std::cout << "I'm getImageMetaData END!!!!!!!!!!!!" << std::endl;
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

    pbnjson::JValue request = generateLunaPayload(MediaIndexerClientAPI::RequestDelete, uri);
    std::string ret = mediaDBConnector_->sendDelMessage(request.stringify());
    std::cout << "Return value_ in requestDelete : " << ret << std::endl;
    std::cout << "I'm requestDelete END!!!!!!!!!!!!" << std::endl;
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

    std::string url = indexerConnector_->getIndexerUrl();
    url.append(std::string("requestMediaScan"));
    auto request = pbnjson::Object();
    request.put("path", path);
    std::cout << "url : " << url << " request : " << request.stringify() << std::endl;

    std::string ret = indexerConnector_->sendMessage(url, request.stringify());
    std::cout << "Return value_ in requestMediaScan : " << ret << std::endl;
    std::cout << "I'm MediaIndexerClient::requestMediaScan END!!!!!!!!!!!!" << std::endl;
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
            if (uri.empty()) {
                auto where = prepareWhere("dirty", false, true);
                query = prepareQuery(selectArray, audioKind_, where);
            } else {
                auto where = prepareWhere("uri", uri, false);
                auto filter = prepareWhere("dirty", false, true);
                query = prepareQuery(selectArray, audioKind_, where, filter);
            }
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
            if (uri.empty()) {
                auto where = prepareWhere("dirty", false, true);
                query = prepareQuery(selectArray, videoKind_, where);
            } else {
                auto where = prepareWhere("uri", uri, false);
                auto filter = prepareWhere("dirty", false, true);
                query = prepareQuery(selectArray, videoKind_, where, filter);
            }
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
            if (uri.empty()) {
                auto where = prepareWhere("dirty", false, true);
                query = prepareQuery(selectArray, imageKind_, where);
            } else {
                auto where = prepareWhere("uri", uri, false);
                auto filter = prepareWhere("dirty", false, true);
                query = prepareQuery(selectArray, imageKind_, where, filter);
            }
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

            auto where = prepareWhere("uri", uri, false);
            auto filter = prepareWhere("dirty", false, true);
            auto query = prepareQuery(selectArray, audioKind_, where, filter);
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

            auto where = prepareWhere("uri", uri, false);
            auto filter = prepareWhere("dirty", false, true);
            auto query = prepareQuery(selectArray, videoKind_, where, filter);
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

            auto where = prepareWhere("uri", uri, false);
            auto filter = prepareWhere("dirty", false, true);
            auto query = prepareQuery(selectArray, imageKind_, where, filter);
            request.put("query", query);
            break;
        }
        case MediaIndexerClientAPI::RequestDelete: {
            auto where = prepareWhere("uri", uri, false);
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

    kindId = typeFromMime(contentType);
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

pbnjson::JValue MediaIndexerClient::prepareWhere(const std::string &key,
                                                 const std::string &value,
                                                 bool precise,
                                                 pbnjson::JValue whereClause) const
{
    auto cond = pbnjson::Object();
    cond.put("prop", key);
    cond.put("op", precise ? "=" : "%");
    cond.put("val", value);
    whereClause << cond;
    return whereClause;
}

pbnjson::JValue MediaIndexerClient::prepareWhere(const std::string &key,
                                                 bool value,
                                                 bool precise,
                                                 pbnjson::JValue whereClause) const
{
    auto cond = pbnjson::Object();
    cond.put("prop", key);
    cond.put("op", precise ? "=" : "%");
    cond.put("val", value);
    whereClause << cond;
    return whereClause;
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

pbnjson::JValue MediaIndexerClient::prepareQuery(pbnjson::JValue selectArray,
                                                 const std::string& kindId,
                                                 pbnjson::JValue where,
                                                 pbnjson::JValue filter) const
{
    auto query = pbnjson::Object();
    query.put("select", selectArray);
    query.put("from", kindId);
    query.put("where", where);
    query.put("filter", filter);
    return query;
}

std::string MediaIndexerClient::getDeviceList() const
{
    if (!indexerConnector_) {
        std::cout << "indexerConnector is NULL!" << std::endl;
        return std::string();
    }

    std::cout << "I'm getDeviceList" << std::endl;
    std::cout << "thread id[" << std::this_thread::get_id() << "]" << std::endl;

    std::string url = indexerConnector_->getIndexerUrl();
    url.append(std::string("getDeviceList"));
    auto request = pbnjson::Object();
    request.put("subscribe", true);
    std::cout << "url : " << url << "request : " << request.stringify() << std::endl;

    std::string ret = indexerConnector_->sendMessage(url, request.stringify());
    std::cout << "Return value_ in getDeviceList : " << ret << std::endl;
    std::cout << "I'm getDeviceList END!!!!!!!!!!!!" << std::endl;
    return ret;
}

void MediaIndexerClient::getMediaDBPermission() const
{
    if (!mediaDBConnector_ || !indexerConnector_)
        return;

    std::cout << "I'm MediaIndexerClient::getMediaDBPermission" << std::endl;
    std::cout << "thread id[" << std::this_thread::get_id() << "]" << std::endl;

    std::string url = indexerConnector_->getIndexerUrl();
    url.append(std::string("getMediaDbPermission"));
    auto request = pbnjson::Object();
    request.put("serviceName", mediaDBConnector_->getServiceName());
    std::cout << "url : " << url << " request : " << request.stringify() << std::endl;

    std::string ret = indexerConnector_->sendMessage(url, request.stringify());
    std::cout << "Return value_ in getMediaDBPermission : " << ret << std::endl;
    std::cout << "I'm MediaIndexerClient::getMediaDBPermission END!!!!!!!!!!!!" << std::endl;
}
