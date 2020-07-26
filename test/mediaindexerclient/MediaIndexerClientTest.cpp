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
#include "mediaindexerclient.h"

#define UNUSED(expr) do { (void)(expr); } while(0)

enum class MediaIndexerAPI : int {
    GET_AUDIO_LIST = 1,
    GET_VIDEO_LIST,
    GET_IMAGE_LIST,
    GET_AUDIO_META_DATA,
    GET_VIDEO_META_DATA,
    GET_IMAGE_META_DATA
};

std::map<MediaIndexerAPI, std::string> indexerMenu = {
    { MediaIndexerAPI::GET_AUDIO_LIST,      std::string("getAudioList") },
    { MediaIndexerAPI::GET_VIDEO_LIST,      std::string("getVideoList") },
    { MediaIndexerAPI::GET_IMAGE_LIST,      std::string("getImageList") },
    { MediaIndexerAPI::GET_AUDIO_META_DATA, std::string("getAudioMetaData") },
    { MediaIndexerAPI::GET_VIDEO_META_DATA, std::string("getVideoMetaData") },
    { MediaIndexerAPI::GET_IMAGE_META_DATA, std::string("getImageMetaData") }
};

static void printMenu(void)
{
    std::cout << std::string("*** Media Indexer Client Command test ***") << std::endl;
    for (auto it : indexerMenu)
        std::cout << static_cast<int>(it.first) << ". " << it.second << std::endl;
    std::cout << std::string("Enter a number(q to quit): ");
}

static bool processCommand(const std::string& cmd, MediaIndexerClient &client, GMainLoop* loop)
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
            std::string ret = client.getAudioList();
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_VIDEO_LIST: {
            // call MediaIndexerClient API
            std::string ret = client.getVideoList();
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_IMAGE_LIST: {
            // call MediaIndexerClient API
            std::string ret = client.getImageList();
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_AUDIO_META_DATA: {
            // call MediaIndexerClient API
            std::string ret = client.getAudioMetaData(std::string());
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_VIDEO_META_DATA: {
            // call MediaIndexerClient API
            std::string ret = client.getVideoMetaData(std::string());
            std::cout << ret << std::endl;
            break;
        }
        case MediaIndexerAPI::GET_IMAGE_META_DATA: {
            // call MediaIndexerClient API
            std::string ret = client.getImageMetaData(std::string());
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
    std::cout << "event : " << event << std::endl;
    std::cout << "thread[" << std::this_thread::get_id() << "]" << std::endl;
}

int main(int argc, char* argv[])
{
    std::cout << "main thread[" << std::this_thread::get_id() << "]" << std::endl;
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    std::thread th([](GMainLoop *loop) -> void {
        std::cout << "thread[" << std::this_thread::get_id() << "]" << std::endl;
        MediaIndexerClient client(my_indexer_callback);
        while(1) {
            printMenu();
            std::string cmd;
            std::getline(std::cin, cmd);
            bool ret = processCommand(cmd, client, loop);
            if (!ret) {
                std::cout << std::string("Exit thread!") << std::endl;
                break;
            }
        }
    }, loop);

    g_main_loop_run(loop);
    th.join();
    g_main_loop_unref(loop);
    std::cout << std::string("Exit Client program!") << std::endl;
    return 0;
}
