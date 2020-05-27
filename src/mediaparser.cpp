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

#include "mediaparser.h"
#include "plugins/pluginfactory.h"
#include "plugins/plugin.h"
#include "metadataextractors/imetadataextractor.h"

std::queue<std::unique_ptr<MediaParser>> MediaParser::tasks_;
std::map<std::pair<MediaItem::Type, std::string>, std::unique_ptr<IMetaDataExtractor>> MediaParser::extractor_;
int MediaParser::runningThreads_ = 0;
std::mutex MediaParser::lock_;

void MediaParser::enqueueTask(MediaItemPtr mediaItem)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto ext = mediaItem->ext();
    if (extractor_.empty()) {
        for (auto type = MediaItem::Type::Audio;
             type < MediaItem::Type::EOL; ++type) {
            LOG_DEBUG("Extractor is added for type = %d, ext = %s", type, ext.c_str());
            std::pair<MediaItem::Type, std::string> p(type, ext);
            extractor_[p] = std::move(IMetaDataExtractor::extractor(type, ext));
        }
    }

    tasks_.push(std::make_unique<MediaParser>(std::move(mediaItem)));

    // let's try to start the task and any other pending tasks
    runTask();
}

MediaParser::~MediaParser()
{
    // all threads are running in detached mode so no need to cleanup
    // anything here
}

MediaParser::MediaParser(MediaItemPtr mediaItem) :
    mediaItem_(std::move(mediaItem)),
    useDefaultExtractor_(false)
{
    auto path = mediaItem_->path();
    if (*path.begin() == '/')
        useDefaultExtractor_ = true;
    else
        LOG_DEBUG("Use plugin specific meta data extractor for '%s'",
            mediaItem_->uri().c_str());
}

void MediaParser::runTask()
{
    // start as many tasks as possible
    while (runningThreads_ < PARALLEL_META_EXTRACTION && !tasks_.empty()) {
        auto task = std::move(tasks_.front());
        tasks_.pop(); // remove task from queue

        // run the meta data extraction
        auto mp = task.release();
        std::thread t(&MediaParser::extractMeta, mp);
        t.detach(); // runnable

        runningThreads_++;

        LOG_DEBUG("Media parser threads %i", runningThreads_);
    }
}

void MediaParser::extractMeta() const
{
    // make sure we are deleted when this method terminates
    std::unique_ptr<const MediaParser> me(this);

    auto mi = mediaItem_.get();
    LOG_DEBUG("Media item to extract %p with parser %p", mi, this);

    if (useDefaultExtractor_) {
        std::pair<MediaItem::Type, std::string> p(mediaItem_->type(), mi->ext());

        if (extractor_.find(p) != extractor_.end())
            extractor_[p]->extractMeta(*mi);
        else
        {
            LOG_ERROR(0, "Could not found valid extractor");
            return;
        }
    } else {
        auto plg = PluginFactory().plugin(mediaItem_->uri());
        plg->extractMeta(*mi);
    }

    // if we succeeded push the media item back to observer
    if (mediaItem_->parsed()) {
        LOG_DEBUG("Pushing parsed mediaitem %p back to observer", mi);
        mediaItem_->observer()->newMediaItem(std::move(mediaItem_));
    }

    // update the object state and try to run pending tasks
    std::lock_guard<std::mutex> lock(lock_);
    runningThreads_--;
    runTask(); // let's give it a try

    LOG_DEBUG("Media parser threads %i", runningThreads_);
}
