// Copyright (c) 2019-2020 LG Electronics, Inc.
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


#include <pbnjson.hpp>
#include <luna-service2/lunaservice.h>
#include <string>
#include <mutex>
#include <iostream>
#include <queue>

class IndexerServiceMock
{
public:
    IndexerServiceMock(std::string serviceName, GMainLoop* loop);
    ~IndexerServiceMock();
    void run();

private:

    /// Callback for getDeviceList() Luna Method.
    static bool onDeviceListGet(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    /// Callback for getMediaDbPermission() Luna Method.
    static bool onMediaDbPermissionGet(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    /// Callback for getList for audio/video/image Luna Method.
    static bool onGetAudioList(LSHandle *lsHandle, LSMessage *msg, void *ctx);
    static bool onGetVideoList(LSHandle *lsHandle, LSMessage *msg, void *ctx);
    static bool onGetImageList(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    /// Callback for getMetaData for audio/video/image Luna Method.
    static bool onGetAudioMetaData(LSHandle *lsHandle, LSMessage *msg, void *ctx);
    static bool onGetVideoMetaData(LSHandle *lsHandle, LSMessage *msg, void *ctx);
    static bool onGetImageMetaData(LSHandle *lsHandle, LSMessage *msg, void *ctx);

    static int sendResponse(void *ctx);

    /// Service method definitions.
    static LSMethod serviceMethods_[];

//    bool notifySubscribers(int eventType);

    /// Luna service handle.
    LSHandle *lsHandle_;

    std::string serviceName_;
    GMainLoop* mainLoop_;

    std::queue<std::string> queue_;
};
