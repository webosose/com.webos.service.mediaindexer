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

#pragma once

#include "task.h"
#include "mediaitem.h"
#include "metadataextractors/imetadataextractor.h"
#include <pbnjson.hpp>

#include <thread>
#include <mutex>
#include <queue>
#include <list>
#include <atomic>

/// Media parser class for meta data extraction.
class MediaParser
{
 public:
    /// Enqueue meta data extraction task.
    static void enqueueTask(MediaItemPtr mediaItem);

    /// Get media parser instance for certain device.
    MediaParser(MediaItemPtr mediaItem);

    /// Get media parser instance for direct metadata extraction
    MediaParser(std::string &uri);

    /// For the purpose of direct meta extraction from indexer service api
    bool extractMetaDirect(pbnjson::JValue &meta);
    virtual ~MediaParser();

 private:
    /// Get message id.
    LOG_MSGID;

    /// Construction is only allowed with media item.
    MediaParser() {};

    /// Start new task, must be called with lock locked.
    static void runTask();

    /// Queue of meta data extraction tasks.
    static std::queue<std::unique_ptr<MediaParser>> tasks_;
    /// Number of currently running threads.
    static int runningThreads_;
    /// Make class static data thread safe.
    static std::mutex lock_;
    /// Meta data extrator.
    static std::map<std::pair<MediaItem::Type, std::string>,
           std::unique_ptr<IMetaDataExtractor>> extractor_;

    static std::map<MediaItem::Type, Task> taskMap_;

    /// Do the meta data extraction.
    void extractMeta() const;

    /// The media item this media parser works on - extractMeta will
    /// modify it internally though it is otherwise consideren to be
    /// const so better make this mutable
    mutable MediaItemPtr mediaItem_;
    /// Set if the default extractor shall be used.
    bool useDefaultExtractor_;
};
