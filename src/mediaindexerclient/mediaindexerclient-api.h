/* Copyright (c) 2020 LG Electronics, Inc.
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

#ifndef MEDIA_INDEXER_CLIENT_API_H_
#define MEDIA_INDEXER_CLIENT_API_H_

#include <stdint.h>
#include <stddef.h>
#include <functional>
#include <memory>
#include <string>
#include "mediaindexer-common.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef void* MediaIndexerHandle;
    typedef std::function<void(MediaIndexerClientEvent event, void* clientData, void* userData)> MediaIndexerCallback;

    /**
     *  Create media indexer client
     */
    MediaIndexerHandle CreateMediaIndexerClient(MediaIndexerCallback callback, void* userData);

    /**
     * Destory media indexer client
     */
    void DestroyMediaIndexerClient(MediaIndexerHandle handle);

    /**
     * Get Connected Device list
     */
    std::string GetDeviceList(MediaIndexerHandle handle);

    /**
     * Get Audio list given uri
     */
    std::string GetAudioList(MediaIndexerHandle handle, const std::string& uri);

    /**
     * Get Video list given uri
     */
    std::string GetVideoList(MediaIndexerHandle handle, const std::string& uri);

    /**
     * Get Image list given uri
     */
    std::string GetImageList(MediaIndexerHandle handle, const std::string& uri);


    /**
     * Get Audio meta data given uri
     */
    std::string GetAudioMetaData(MediaIndexerHandle handle, const std::string& uri);

    /**
     * Get Video meta data given uri
     */
    std::string GetVideoMetaData(MediaIndexerHandle handle, const std::string& uri);

    /**
     * Get Image meta data given uri
     */
    std::string GetImageMetaData(MediaIndexerHandle handle, const std::string& uri);

    /**
     * Request to delete data given uri
     */
    std::string RequestDelete(MediaIndexerHandle handle, const std::string& uri);

    /**
     * Request to media scan given path
     */
    std::string RequestMediaScan(MediaIndexerHandle handle, const std::string& path);

#ifdef __cplusplus
}
#endif

#endif  // MEDIA_INDEXER_CLIENT_API_H_
