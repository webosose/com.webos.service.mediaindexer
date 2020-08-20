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
#include "dbconnector/mediadb.h"
#include <thread>
#include <chrono>
#include <condition_variable>


std::queue<std::unique_ptr<MediaParser>> MediaParser::tasks_;
std::map<std::pair<MediaItem::Type, std::string>, std::unique_ptr<IMetaDataExtractor>> MediaParser::extractor_;
int MediaParser::runningThreads_ = 0;
std::mutex MediaParser::lock_;
std::unique_ptr<MediaParser> MediaParser::instance_;
std::mutex MediaParser::ctorLock_;

typedef struct MediaItemWrapper
{
    MediaItemPtr mediaItem_;
} MediaItemWrapper_t;

void MediaParser::enqueueTask(MediaItemPtr mediaItem)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto ext = mediaItem->ext();
    auto type = mediaItem->type();
    std::pair<MediaItem::Type, std::string> p(type, ext);
    if (extractor_.find(p) == extractor_.end()) {
        LOG_DEBUG("Extractor is added for type = %d, ext = %s", static_cast<int>(type), ext.c_str());
        extractor_[p] = std::move(IMetaDataExtractor::extractor(type, ext));
    }

    MediaParser* mParser = MediaParser::instance();
    MediaItemWrapper_t *mp = new MediaItemWrapper_t;
    mp->mediaItem_ = std::move(mediaItem);
    GError *error = nullptr;
    if (!g_thread_pool_push(mParser->pool, (void *)(mp), &error)) {
        LOG_ERROR(0, "Fail occurred in g_thread_pool_push");
        if (error) {
            LOG_ERROR(0, "Error Message : %s", error->message);
            g_error_free(error);
        }
    }
}

MediaParser *MediaParser::instance()
{
    std::lock_guard<std::mutex> lk(ctorLock_);
    if (!instance_.get()) {
        instance_.reset(new MediaParser());
    }
    return instance_.get();
}

MediaParser::~MediaParser()
{
    // all threads are running in detached mode so no need to cleanup
    // anything here
    LOG_INFO(0, "MediaParser Dtor!!!");
    g_thread_pool_free(pool, TRUE, TRUE);
    //mediaItem_.reset();
}

MediaParser::MediaParser()
{
    pool = g_thread_pool_new((GFunc) &MediaParser::extractMeta, this, PARALLEL_META_EXTRACTION, TRUE, NULL);
    g_thread_pool_set_max_unused_threads(PARALLEL_META_EXTRACTION);
}

bool MediaParser::setMediaItem(std::string & uri)
{
    std::lock_guard<std::mutex> lock(mediaItemLock_);
    if (mediaItem_.get() != nullptr)
        mediaItem_.reset();
    mediaItem_ = std::unique_ptr<MediaItem>(new MediaItem(uri));
    if (mediaItem_.get() == nullptr) {
        LOG_ERROR(0, "Failed to get mediaitem!");
        return false;
    }
    return true;
}

bool MediaParser::extractMetaDirect(pbnjson::JValue &meta)
{
    try {
        std::lock_guard<std::mutex> lock(mediaItemLock_);
        auto mi = mediaItem_.get();
        if (mi == nullptr) {
            LOG_ERROR(0, "Media Item is invalid");
            return false;
        }
        LOG_DEBUG("Media item to extract %p with parser %p", mi, this);
        auto path = mediaItem_->path();

        if (*path.begin() == '/') {
            std::pair<MediaItem::Type, std::string> p(mediaItem_->type(), mi->ext());

            if (extractor_.find(p) != extractor_.end()) {
                extractor_[p]->extractMeta(*mi, true);
                mi->setParsed(true);
            } else {
                LOG_WARNING(0, "Could not found valid extractor, type : %s, ext : %s", MediaItem::mediaTypeToString(mediaItem_->type()).c_str(), mi->ext().c_str());
                LOG_DEBUG("Create new extractor");
                extractor_[p] = std::move(IMetaDataExtractor::extractor(p.first, p.second));
                extractor_[p]->extractMeta(*mi, true);
                mi->setParsed(true);
            }
        } else {
            auto plg = PluginFactory().plugin(mediaItem_->uri());
            plg->extractMeta(*mi, true);
            mi->setParsed(true);
        }
        LOG_DEBUG("Start mediaItem_->putAllMetaToJson");
        if (!mediaItem_->putAllMetaToJson(meta)) {
            LOG_ERROR(0, "Failed to put meta to json");
            return false;
        }
        mediaItem_.reset();
    } catch (const std::exception & e) {
        LOG_ERROR(0, "MediaParser::extractMeta failure: %s", e.what());
        return false;
    } catch (...) {
        LOG_ERROR(0, "MediaParser::extractMeta failure by unexpected failure");
        return false;
    }
    return true;
}

void MediaParser::extractMeta(void *data, void *user_data)
{
    // make sure we are deleted when this method terminates
    try {
        if (!data || !user_data) {
            LOG_ERROR(0, "Invalid Input parameters");
            return;
        }
        MediaParser *mp = static_cast<MediaParser *>(user_data);
        MediaItemWrapper_t * pMip = static_cast<MediaItemWrapper_t *>(data);
        if (!pMip) {
            LOG_ERROR(0, "Failed to extractor media item, Invalid input data parameter");
            return;
        }
        MediaItemPtr mip = std::move(pMip->mediaItem_);
        LOG_INFO(0, "Media item to extract %p with parser %p", mip.get(), mp);

        auto path = mip->path();
        if (*path.begin() == '/') {
            std::pair<MediaItem::Type, std::string> p(mip->type(), mip->ext());

            if (extractor_.find(p) != extractor_.end()) {
                extractor_[p]->extractMeta(*mip);
                mip->setParsed(true);
            } else {
                LOG_WARNING(0, "Could not found valid extractor, type : %s, ext : %s", MediaItem::mediaTypeToString(mip->type()).c_str(), mip->ext().c_str());
                LOG_DEBUG("Create new extractor");
                extractor_[p] = std::move(IMetaDataExtractor::extractor(p.first, p.second));
                extractor_[p]->extractMeta(*mip);
                mip->setParsed(true);
            }
        } else {
            auto plg = PluginFactory().plugin(mip->uri());
            plg->extractMeta(*mip);
            mip->setParsed(true);
        }
        // if we succeeded push the media item back to observer
        if (mip->parsed()) {
            LOG_INFO(0, "Pushing parsed mediaitem %p back to observer, newMediaItem start", mip.get());
            auto mdb = MediaDb::instance();
            mdb->updateMediaItem(std::move(mip));
            LOG_INFO(0, "mdb->updateMediaItem Done");
        }
        delete pMip;
    } catch (const std::exception & e) {
        LOG_ERROR(0, "MediaParser::extractMeta failure: %s", e.what());
    } catch (...) {
        LOG_ERROR(0, "MediaParser::extractMeta failure by unexpected failure");
    }
}
