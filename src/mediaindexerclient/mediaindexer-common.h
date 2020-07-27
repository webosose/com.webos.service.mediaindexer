/* Copyright (c) 2018-2020 LG Electronics, Inc.
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

#ifndef MEDIA_INDEXER_CLINT_COMMON_H_
#define MEDIA_INDEXER_CLINT_COMMON_H_
/**
 * notification events that the callback function should handle
 */
enum class MediaIndexerClientEvent : int {
    NotifyGetAudioList,
    NotifyGetVideoList,
    NotifyGetImageList,
    NotifyGetAudioMetaData,
    NotifyGetVideoMetaData,
    NotifyGetImageMetaData,
    EOL
};

/**
 * Media indexer client API specifiers.
 */
enum class MediaIndexerClientAPI : int {
    GetAudioListAPI,
    GetVideoListAPI,
    GetImageListAPI,
    GetAudioMetaDataAPI,
    GetVideoMetaDataAPI,
    GetImageMetaDataAPI,
    EOL
};

#endif  // MEDIA_INDEXER_CLINT_COMMON_H_
