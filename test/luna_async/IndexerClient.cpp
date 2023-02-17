/* Copyright (c) 2019-2020 LG Electronics, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <thread>
#include <memory>
#include <string>
#include <functional>
#include <map>
#include <glib.h>
#include "IndexerClient.h"

#define UNUSED(expr) do { (void)(expr); } while(0)
enum class MediaIndexerAPI : int {
    GET_AUDIO_LIST = 1,
    GET_VIDEO_LIST,
    GET_IMAGE_LIST,
    GET_AUDIO_META_DATA,
    GET_VIDEO_META_DATA,
    GET_IMAGE_META_DATA,
    GET_DEVICE_LIST,
    GET_MEDIA_DB_PERMISSION
};

std::map<MediaIndexerAPI, std::string> indexerMenu = {
    { MediaIndexerAPI::GET_AUDIO_LIST,      std::string("getAudioList")  },
    { MediaIndexerAPI::GET_VIDEO_LIST,      std::string("getVideoList")  },
    { MediaIndexerAPI::GET_IMAGE_LIST,      std::string("getImageList")  },
    { MediaIndexerAPI::GET_AUDIO_META_DATA, std::string("getAudioMetaData") },
    { MediaIndexerAPI::GET_VIDEO_META_DATA, std::string("getVideoMetaData") },
    { MediaIndexerAPI::GET_IMAGE_META_DATA, std::string("getImageMetaData") },
    { MediaIndexerAPI::GET_DEVICE_LIST,     std::string("getDeviceList") },
    { MediaIndexerAPI::GET_MEDIA_DB_PERMISSION,   std::string("getMediaDBPermission")}
};

IndexerClientMock::IndexerClientMock(std::string& name, GMainLoop* loop)
    : lsHandle_(nullptr)
    , mainLoop_(loop)
    , serviceName_(name)
    , serverUrl_("luna://com.webos.service.mediaindexerMock/")
{
    std::cout << std::string("IndexerClientMock ctor!") << std::endl;
    LSError lsError;
    LSErrorInit(&lsError);

    if (!LSRegister(serviceName_.c_str(), &lsHandle_, &lsError)) {
        std::cout << std::string("Unable to register at luna-bus") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return;
    }

    if (!LSRegisterCategory(lsHandle_, "/", nullptr, nullptr, nullptr, &lsError)) {
        std::cout << std::string("Unable to register top level category") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return;
    }

    if (!LSCategorySetData(lsHandle_, "/", this, &lsError)) {
        std::cout << std::string("Unable to set data on top level category") << std::endl    ;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return;
    }

    if (!LSGmainAttach(lsHandle_, mainLoop_, &lsError)) {
        std::cout << std::string("Unable to attach service") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return;
    }

    LSErrorFree(&lsError);
}

IndexerClientMock::~IndexerClientMock()
{
    if (!lsHandle_) return;
    LSError lsError;
    LSErrorInit(&lsError);

    if (!LSUnregister(lsHandle_, &lsError)) {
        std::cout << std::string("Service unregister failed") << std::endl;
        LSErrorPrint(&lsError, stdout);
    }

    LSErrorFree(&lsError);
}

bool IndexerClientMock::getAudioList()
{
    auto subscription = pbnjson::Object();

    subscription.put("subscribe", true);

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = serverUrl_ + std::string("getAudioList");

    if (!LSCall(lsHandle_, url.c_str(), subscription.stringify().c_str(),
                IndexerClientMock::onIndexerNotification, this, &sessionToken,
                &lsError)) {
        std::cout << std::string("Service subscription error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return false;
    }

    return true;
}

bool IndexerClientMock::getVideoList()
{
    auto subscription = pbnjson::Object();

    subscription.put("subscribe", true);

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = serverUrl_ + std::string("getVideoList");

    if (!LSCall(lsHandle_, url.c_str(), subscription.stringify().c_str(),
                IndexerClientMock::onIndexerNotification, this, &sessionToken,
                &lsError)) {
        std::cout << std::string("Service subscription error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return false;
    }

    return true;
}
bool IndexerClientMock::getImageList()
{
    auto subscription = pbnjson::Object();

    subscription.put("subscribe", true);

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = serverUrl_ + std::string("getImageList");

    if (!LSCall(lsHandle_, url.c_str(), subscription.stringify().c_str(),
                IndexerClientMock::onIndexerNotification, this, &sessionToken,
                &lsError)) {
        std::cout << std::string("Service subscription error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return false;
    }

    return true;

}
bool IndexerClientMock::getAudioMetaData()
{
    auto subscription = pbnjson::Object();

    subscription.put("subscribe", true);

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = serverUrl_ + std::string("getAudioMetaData");

    if (!LSCall(lsHandle_, url.c_str(), subscription.stringify().c_str(),
                IndexerClientMock::onIndexerNotification, this, &sessionToken,
                &lsError)) {
        std::cout << std::string("Service subscription error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return false;
    }

    return true;

}
bool IndexerClientMock::getVideoMetaData()
{
    auto subscription = pbnjson::Object();

    subscription.put("subscribe", true);

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = serverUrl_ + std::string("getVideoMetaData");

    if (!LSCall(lsHandle_, url.c_str(), subscription.stringify().c_str(),
                IndexerClientMock::onIndexerNotification, this, &sessionToken,
                &lsError)) {
        std::cout << std::string("Service subscription error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return false;
    }

    return true;

}
bool IndexerClientMock::getImageMetaData()
{
    auto subscription = pbnjson::Object();

    subscription.put("subscribe", true);

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = serverUrl_ + std::string("getImageMetaData");

    if (!LSCall(lsHandle_, url.c_str(), subscription.stringify().c_str(),
                IndexerClientMock::onIndexerNotification, this, &sessionToken,
                &lsError)) {
        std::cout << std::string("Service subscription error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return false;
    }

    return true;

}
bool IndexerClientMock::getDeviceList()
{
    auto subscription = pbnjson::Object();

    subscription.put("subscribe", true);

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = serverUrl_ + std::string("getDeviceList");

    if (!LSCall(lsHandle_, url.c_str(), subscription.stringify().c_str(),
                IndexerClientMock::onIndexerNotification, this, &sessionToken,
                &lsError)) {
        std::cout << std::string("Service subscription error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return false;
    }

    return true;

}
bool IndexerClientMock::getMediaDBPermission()
{
    auto subscription = pbnjson::Object();

    subscription.put("subscribe", true);

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    std::string url = serverUrl_ + std::string("getMediaDBPermission");

    if (!LSCall(lsHandle_, url.c_str(), subscription.stringify().c_str(),
                IndexerClientMock::onIndexerNotification, this, &sessionToken,
                &lsError)) {
        std::cout << std::string("Service subscription error") << std::endl;
        LSErrorPrint(&lsError, stdout);
        LSErrorFree(&lsError);
        return false;
    }

    return true;

}

bool IndexerClientMock::onIndexerNotification(LSHandle *lsHandle, 
                                              LSMessage *msg, 
                                              void *ctx)
{
    IndexerClientMock *mock = static_cast<IndexerClientMock *>(ctx);

    const char *payload = LSMessageGetPayload(msg);
    pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());

    if (!parser.parse(payload)) {
        std::cout << std::string("Invalid JSON message: ") << payload << std::endl;
        return false;
    }

    pbnjson::JValue domTree(parser.getDom());
    if (domTree.hasKey("testString")) {
        auto str = domTree["testString"].asString();
        std::cout << std::string("Received from service -> ") << str << std::endl;
    }

    return true;
}


static void printMenu(void)
{
    std::cout << std::string("*** Indexer Client async test ***") << std::endl;
    for (auto it : indexerMenu)
        std::cout << static_cast<int>(it.first) << ". " << it.second << std::endl;
    std::cout << std::string("Enter a number(q to quit): ");
}

static bool processCommand(const std::string& cmd, IndexerClientMock &client, GMainLoop* loop)
{
    if (cmd.compare("q") == 0) {
        g_main_loop_quit(loop);
        return false;
    }

    // get API from user command
    MediaIndexerAPI api = static_cast<MediaIndexerAPI>(std::stoi(cmd));

    switch(api) {
        case MediaIndexerAPI::GET_AUDIO_LIST: {
            // call MediaIndexerClient API
            bool ret = client.getAudioList();
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_VIDEO_LIST: {
            // call MediaIndexerClient API
            bool ret = client.getVideoList();
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_IMAGE_LIST: {
            // call MediaIndexerClient API
            bool ret = client.getImageList();
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_AUDIO_META_DATA: {
            // call MediaIndexerClient API
            bool ret = client.getAudioMetaData();
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_VIDEO_META_DATA: {
            // call MediaIndexerClient API
            bool ret = client.getVideoMetaData();
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_IMAGE_META_DATA: {
            // call MediaIndexerClient API
            bool ret = client.getImageMetaData();
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_DEVICE_LIST: {
            // call MediaIndexerClient API
            bool ret = client.getDeviceList();
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_MEDIA_DB_PERMISSION: {
            // call MediaIndexerClient API
            bool ret = client.getMediaDBPermission();
            std::cout << ret << std::endl;
            break;
        }

        default: {
            std::cout << std::string("Wrong command!") << std::endl;
            break;
        }
    }
    return true;
}
/*
static void indexer_callback(MediaIndexerClientEvent event, void* clientData, void* userData)
{
    std::cout << "indexer_callback" << std::endl;
    std::cout << "event : " << event << std::endl;
}
*/
int main(int argc, char* argv[])
{
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    std::thread th([](GMainLoop *loop) -> void {
        std::string clientName("com.webos.service.mediaindexerMockClient");
        IndexerClientMock client(clientName, loop);
        while(1) {
            printMenu();
            std::string cmd;
            std::getline(std::cin, cmd);
            if(!cmd.empty())
            {
              bool ret = processCommand(cmd, client, loop);
              if (!ret) {
                std::cout << std::string("Exit thread!") << std::endl;
                break;
              }
            }
        }
    }, loop);

    g_main_loop_run(loop);
    th.join();
    g_main_loop_unref(loop);
    std::cout << std::string("Exit Client program!") << std::endl;
    return 0;
}
