// Copyright (c) 2018-2020 LG Electronics, Inc.
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

#include <string>
#include <memory>
#include <mutex>
#include "lunaconnector.h"
#include "mediaindexer-common.h"


class MediaIndexerClient {
public:
    using MediaIndexerCallback = std::function<void(MediaIndexerClientEvent event, void* clientData, void* userData)>;
    MediaIndexerClient(MediaIndexerCallback cb = nullptr, void* userData = nullptr);
    ~MediaIndexerClient();

    // Media Indexer Client API
    std::string getDeviceList();

    // Media Indexer Client Database API
    std::string getAudioList(const std::string& uri = std::string());
    std::string getVideoList(const std::string& uri = std::string());
    std::string getImageList(const std::string& uri = std::string());
    std::string getAudioMetaData(const std::string& uri);
    std::string getVideoMetaData(const std::string& uri);
    std::string getImageMetaData(const std::string& uri);

private:
    static constexpr const char *dbUrl_ = "luna://com.webos.service.db/";
    static constexpr const char *mediaKind_ = "com.webos.service.mediaindexer.media:1";
    static constexpr const char *audioKind_ = "com.webos.service.mediaindexer.audio:1";
    static constexpr const char *videoKind_ = "com.webos.service.mediaindexer.video:1";
    static constexpr const char *imageKind_ = "com.webos.service.mediaindexer.image:1";

    MediaIndexerCallback callback_;
    void* userData_;

    std::mutex mutex_;
    std::string returnValue_;

    LSHandle* lsHandle_;
    GMainLoop* loop_;
    GMainContext* context_;
    std::thread task_;

    static bool onGetDeviceList(LSHandle* lsHandle, LSMessage* msg, void* ctx);
    bool handleResponseFromIndexer(LSMessage* msg);

    // Luna connector handle.
    std::unique_ptr<LunaConnector> dbConnector_;
    std::unique_ptr<LunaConnector> indexerConnector_;

    // Luna response callback and handler.
    static bool onLunaResponse(LSHandle* lsHandle, LSMessage* msg, void* ctx);
    bool handleLunaResponse(LSMessage* msg);

    pbnjson::JValue generateLunaPayload(MediaIndexerClientAPI api, const std::string& uri);

    // prevent copy & assign operation
    MediaIndexerClient(MediaIndexerClient const&) = delete;
    MediaIndexerClient(MediaIndexerClient &&) = delete;
    MediaIndexerClient& operator =(MediaIndexerClient const&) = delete;
    MediaIndexerClient& operator =(MediaIndexerClient &&) = delete;
};
