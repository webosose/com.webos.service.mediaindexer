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

#include "mediaindexerclient-api.h"
#include "mediaindexerclient.h"
#include "logging.h"
#include <iostream>
#include <string>

struct IndexerClientWrapper {
    IndexerClientWrapper(MediaIndexerCallback callback, void* userData) {
        client_ = new MediaIndexerClient(callback, userData);
    };
    MediaIndexerClient* client_;
};

MediaIndexerHandle CreateMediaIndexerClient(MediaIndexerCallback callback, void* userData)
{
    std::cout << "Create MediaIndexerClient" << std::endl;
    IndexerClientWrapper* indexerWrapper = new IndexerClientWrapper(callback, userData);
    return static_cast<MediaIndexerHandle>(indexerWrapper);
}

void DestroyMediaIndexerClient(MediaIndexerHandle handle)
{
    std::cout << "Destory MediaIndexerClient" << std::endl;
    if (!handle) {
        std::cout << std::string("MediaIndexerHandle is NULL!") << std::endl;
        return;
    }
    IndexerClientWrapper* indexerWrapper = static_cast<IndexerClientWrapper*>(handle);
    delete indexerWrapper;
}

std::string GetAudioList(MediaIndexerHandle handle, std::string uri)
{
    std::cout << std::string("GetAudioList") << std::endl;
    if (!handle) {
        std::cout << std::string("MediaIndexerHandle is NULL!") << std::endl;
        return std::string();
    }
    IndexerClientWrapper* indexerWrapper = static_cast<IndexerClientWrapper*>(handle);
    return indexerWrapper->client_->getAudioList(uri);
}

std::string GetVideoList(MediaIndexerHandle handle, std::string uri)
{
    std::cout << std::string("GetVideoList") << std::endl;
    if (!handle) {
        std::cout << std::string("MediaIndexerHandle is NULL!") << std::endl;
        return std::string();
    }
    IndexerClientWrapper* indexerWrapper = static_cast<IndexerClientWrapper*>(handle);
    return indexerWrapper->client_->getVideoList(uri);
}

std::string GetImageList(MediaIndexerHandle handle, std::string uri)
{
    std::cout << std::string("GetImageList") << std::endl;
    if (!handle) {
        std::cout << std::string("MediaIndexerHandle is NULL!") << std::endl;
        return std::string();
    }
    IndexerClientWrapper* indexerWrapper = static_cast<IndexerClientWrapper*>(handle);
    return indexerWrapper->client_->getImageList(uri);
}


std::string GetAudioMetaData(MediaIndexerHandle handle, std::string uri)
{
    std::cout << std::string("GetAudioMetaData") << std::endl;
    if (!handle) {
        std::cout << std::string("MediaIndexerHandle is NULL!") << std::endl;
        return std::string();
    }
    IndexerClientWrapper* indexerWrapper = static_cast<IndexerClientWrapper*>(handle);
    return indexerWrapper->client_->getAudioMetaData(uri);
}

std::string GetVideoMetaData(MediaIndexerHandle handle, std::string uri)
{
    std::cout << std::string("GetVideoMetaData") << std::endl;
    if (!handle) {
        std::cout << std::string("MediaIndexerHandle is NULL!") << std::endl;
        return std::string();
    }
    IndexerClientWrapper* indexerWrapper = static_cast<IndexerClientWrapper*>(handle);
    return indexerWrapper->client_->getVideoMetaData(uri);
}

std::string GetImageMetaData(MediaIndexerHandle handle, std::string uri)
{
    std::cout << std::string("GetVideoMetaData") << std::endl;
    if (!handle) {
        std::cout << std::string("MediaIndexerHandle is NULL!") << std::endl;
        return std::string();
    }
    IndexerClientWrapper* indexerWrapper = static_cast<IndexerClientWrapper*>(handle);
    return indexerWrapper->client_->getImageMetaData(uri);
}

