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


#include <map>
#include <string>

enum class MediaIndexerLunaAPI : int {
    GET_DEVICE_LIST = 1,
    GET_AUDIO_LIST,
    GET_VIDEO_LIST,
    GET_IMAGE_LIST,
    GET_AUDIO_META_DATA,
    GET_VIDEO_META_DATA,
    GET_IMAGE_META_DATA,
    GET_MEDIA_DB_PERMISSION
};

std::map<MediaIndexerLunaAPI, std::string> indexerMenu = {
    { MediaIndexerLunaAPI::GET_DEVICE_LIST,         std::string("getDeviceList") },
    { MediaIndexerLunaAPI::GET_AUDIO_LIST,          std::string("getAudioList")  },
    { MediaIndexerLunaAPI::GET_VIDEO_LIST,          std::string("getVideoList")  },
    { MediaIndexerLunaAPI::GET_IMAGE_LIST,          std::string("getImageList")  },
    { MediaIndexerLunaAPI::GET_AUDIO_META_DATA,     std::string("getAudioMetaData") },
    { MediaIndexerLunaAPI::GET_VIDEO_META_DATA,     std::string("getVideoMetaData") },
    { MediaIndexerLunaAPI::GET_IMAGE_META_DATA,     std::string("getImageMetaData") },
    { MediaIndexerLunaAPI::GET_MEDIA_DB_PERMISSION, std::string("getMediaDbPermission") }
};
