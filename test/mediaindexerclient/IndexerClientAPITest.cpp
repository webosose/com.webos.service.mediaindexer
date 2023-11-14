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
#include "mediaindexerclient-api.h"

#define UNUSED(expr) do { (void)(expr); } while(0)

enum class MediaIndexerAPI : int {
    GET_DEVICE_LIST = 1,
    GET_AUDIO_LIST,
    GET_VIDEO_LIST,
    GET_IMAGE_LIST,
    GET_AUDIO_META_DATA,
    GET_VIDEO_META_DATA,
    GET_IMAGE_META_DATA,
    REQUEST_DELETE,
    REQUEST_MEDIA_SCAN
};

std::map<MediaIndexerAPI, std::string> indexerMenu = {
    { MediaIndexerAPI::GET_DEVICE_LIST,     std::string("getDeviceList") },
    { MediaIndexerAPI::GET_AUDIO_LIST,      std::string("getAudioList")  },
    { MediaIndexerAPI::GET_VIDEO_LIST,      std::string("getVideoList")  },
    { MediaIndexerAPI::GET_IMAGE_LIST,      std::string("getImageList")  },
    { MediaIndexerAPI::GET_AUDIO_META_DATA, std::string("getAudioMetaData") },
    { MediaIndexerAPI::GET_VIDEO_META_DATA, std::string("getVideoMetaData") },
    { MediaIndexerAPI::GET_IMAGE_META_DATA, std::string("getImageMetaData") },
    { MediaIndexerAPI::REQUEST_DELETE,      std::string("requestDelete") },
    { MediaIndexerAPI::REQUEST_MEDIA_SCAN,  std::string("requestMediaScan") }
};

static void printMenu(void)
{
    std::cout << std::string("*** Media Indexer Client API test ***") << std::endl;
    for (const auto &it : indexerMenu)
        std::cout << static_cast<int>(it.first) << ". " << it.second << std::endl;
    std::cout << std::string("Enter a number(q to quit): ");
}

static bool processCommand(const std::string& cmd, MediaIndexerHandle handle)
{
    if (cmd.compare("q") == 0) {
        return false;
    }

    // get API from user command
    MediaIndexerAPI api = static_cast<MediaIndexerAPI>(std::stoi(cmd));

    switch(api) {
        case MediaIndexerAPI::GET_DEVICE_LIST: {
            // call MediaIndexerClient API
            std::string ret = GetDeviceList(handle);
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_AUDIO_LIST: {
            // call MediaIndexerClient API
            std::cout << "input uri(ex. msc or storage) >> ";
            std::string uri;
            std::getline(std::cin, uri);
            std::string ret = GetAudioList(handle, uri);
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_VIDEO_LIST: {
            // call MediaIndexerClient API
            std::cout << "input uri(ex. msc or storage) >> ";
            std::string uri;
            std::getline(std::cin, uri);
            std::string ret = GetVideoList(handle, uri);
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_IMAGE_LIST: {
            // call MediaIndexerClient API
            std::cout << "input uri(ex. msc or storage) >> ";
            std::string uri;
            std::getline(std::cin, uri);
            std::string ret = GetImageList(handle, uri);
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_AUDIO_META_DATA: {
            // call MediaIndexerClient API
            std::cout << "uri >> ";
            std::string uri;
            std::getline(std::cin, uri);
            std::string ret = GetAudioMetaData(handle, uri);
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_VIDEO_META_DATA: {
            // call MediaIndexerClient API
            std::cout << "uri >> ";
            std::string uri;
            std::getline(std::cin, uri);
            std::string ret = GetVideoMetaData(handle, uri);
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_IMAGE_META_DATA: {
            // call MediaIndexerClient API
            std::cout << "uri >> ";
            std::string uri;
            std::getline(std::cin, uri);
            std::string ret = GetImageMetaData(handle, uri);
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::REQUEST_DELETE: {
            // call MediaIndexerClient API
            std::cout << "uri >> ";
            std::string uri;
            std::getline(std::cin, uri);
            std::string ret = RequestDelete(handle, uri);
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::REQUEST_MEDIA_SCAN: {
            // call MediaIndexerClient API
            std::cout << "path >> ";
            std::string path;
            std::getline(std::cin, path);
            std::string ret = RequestMediaScan(handle, path);
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

static void my_indexer_callback(MediaIndexerClientEvent event, void* clientData, void* userData)
{
    std::cout << "my_indexer_callback" << std::endl;
    std::cout << "event : " << static_cast<int>(event) << std::endl;
    std::cout << "thread[" << std::this_thread::get_id() << "]" << std::endl;
}

// TODO: gTest!!!!!!!!!!!!!!!!!
int main(int argc, char* argv[])
{
    std::cout << "Main start! main thread[" << std::this_thread::get_id() << "]" << std::endl;

    MediaIndexerHandle handle = CreateMediaIndexerClient(my_indexer_callback, nullptr);

    while(1) {
        printMenu();
        std::string cmd;
        std::getline(std::cin, cmd);
        if(!cmd.empty())
        {
          bool ret = processCommand(cmd, handle);
          if (!ret) break;
        }
    }

    DestroyMediaIndexerClient(handle);
    std::cout << std::string("Exit Indexer API test program!") << std::endl;
    return 0;
}
