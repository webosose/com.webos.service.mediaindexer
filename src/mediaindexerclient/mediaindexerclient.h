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

    // Media Indexer Client API
    std::string getAudioList(const std::string& uri = std::string());
    std::string getVideoList(const std::string& uri = std::string());
    std::string getImageList(const std::string& uri = std::string());
    std::string getAudioMetaData(const std::string& uri);
    std::string getVideoMetaData(const std::string& uri);
    std::string getImageMetaData(const std::string& uri);

private:

    static const char *dbUrl_;

    MediaIndexerCallback callback_;
    void* userData_;

    std::mutex mutex_;
    std::string returnValue_;

    // Luna connector handle.
    std::unique_ptr<LunaConnector> connector_;

    // Luna response callback and hanlder.
    static bool onLunaResponse(LSHandle* lsHandle, LSMessage* msg, void* ctx);
    bool handleLunaResponse(LSMessage* msg);

    bool generateLunaPayload(MediaIndexerClientAPI api, const std::string& uri);

    
    // prevent copy & assign operation
    MediaIndexerClient(MediaIndexerClient const&) = delete;
    MediaIndexerClient(MediaIndexerClient &&) = delete;
    MediaIndexerClient& operator =(MediaIndexerClient const&) = delete;
    MediaIndexerClient& operator =(MediaIndexerClient &&) = delete;
};
