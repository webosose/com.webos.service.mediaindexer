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

#include "device.h"
#include "plugins/pluginfactory.h"
#include "plugins/plugin.h"

// Not part of Device class, this is defined at the bottom of device.h
Device::Meta &operator++(Device::Meta &meta)
{
    if (meta == Device::Meta::EOL)
        return meta;
    meta = static_cast<Device::Meta>(static_cast<int>(meta) + 1);
    return meta;
}

std::string Device::metaTypeToString(Device::Meta meta)
{
    switch (meta) {
    case Device::Meta::Name:
        return std::string("name");
    case Device::Meta::Description:
        return std::string("description");
    case Device::Meta::Icon:
        return std::string("icon");
    case Device::Meta::EOL:
        return std::string("eol");
    }

    return "";
}

std::string Device::stateToString(Device::State state)
{
    switch (state) {
    case Device::State::Idle:
        return std::string("idle");
    case Device::State::Scanning:
        return std::string("scanning");
    case Device::State::Inactive:
        return std::string("inactive");
    }

    return "";
}

std::shared_ptr<Device> Device::device(const std::string &uri)
{
    PluginFactory factory;
    auto plg = factory.plugin(uri);
    if (!plg)
        return nullptr;

    if (!plg->hasDevice(uri))
        return nullptr;

    return plg->device(uri);
}

Device::Device(const std::string &uri, int alive, bool avail, std::string uuid) :
    uri_(uri),
    mountpoint_(""),
    uuid_(uuid),
    lastSeen_(),
    state_(Device::State::Inactive),
    available_(avail),
    alive_(alive),
    maxAlive_(alive),
    observer_(nullptr)
{
    lastSeen_ = std::chrono::system_clock::now();
    LOG_DEBUG("Device Ctor, URI : %s UUID : %s, object : %p", uri_.c_str(), uuid_.c_str(), this);
    task_ = std::thread(&Device::scanLoop, this);
    task_.detach();

    cleanUpTask_.create([] (void *ctx, void *data) -> void {
        Device* dev = static_cast<Device *>(ctx);
        if (dev) {
            LOG_DEBUG("Clean Up Task start for device ");
            auto obs = dev->observer();
            if (obs)
                obs->cleanupDevice(dev);
        }        
    });
}

Device::~Device()
{
    // nothing to be done here, the only allocated object is the
    // scanner thread which runs in detached mode and thus does not
    // need any cleanup
    LOG_DEBUG("Device Dtor, URI : %s UUID : %s OBJECT : %p", uri_.c_str(), uuid_.c_str(), this);
    exit_ = true;
    queue_.push_back("");
    cv_.notify_one();
    if (task_.joinable())
        task_.join();
    cleanUpTask_.destroy();
}

bool Device::available(bool check)
{
    if (check) {
        std::unique_lock lock(lock_);
        available_ = checkAlive();
    }

    // if the device becomes unavailable it is likely that the icon
    // uri becomes unavailable as well so better delete the uri and we
    // also need to reset the media item count
    if (!available_) {
        meta_[Device::Meta::Icon] = "";
        resetMediaItemCount();
        setState(State::Inactive, true);
    }

    return available_;
}

bool Device::setAvailable(bool avail)
{
    std::unique_lock lock(lock_);

    auto before = available_;

    available_ = avail;
    if (!available_)
        alive_ = 0;
    else
        resetAlive();

    // if the device becomes unavailable it is likely that the icon
    // uri becomes unavailable as well so better delete the uri and we
    // also need to reset the media item count
    if (!available_) {
        meta_[Device::Meta::Icon] = "";
        resetMediaItemCount();
        setState(State::Inactive, true);
    }

    auto changed = (before != avail);
    if (avail && changed) {
        lastSeen_ = std::chrono::system_clock::now();
        setState(State::Idle, true);
    }

    return changed;
}

const std::string &Device::uri() const
{
    return uri_;
}

const std::string &Device::uuid() const
{
    return uuid_;
}

void Device::setUuid(const std::string &uuid)
{
    std::unique_lock lock(lock_);
    uuid_ = uuid;
}


int Device::alive() const
{
    std::shared_lock lock(lock_);
    return maxAlive_;
}

const std::string &Device::meta(Device::Meta type) const
{
    std::shared_lock lock(lock_);
    return meta_[type];
}

bool Device::setMeta(Device::Meta type, const std::string value)
{
    std::unique_lock lock(lock_);

    // check if something needs to be done
    if (meta_[type] == value)
        return false;

    LOG_DEBUG("Updating meta %i to '%s' for '%s'", (int) type, value.c_str(),
        uri().c_str());
    meta_[type] = value;

    return true;
}

Device::State Device::state() const
{
    std::shared_lock lock(lock_);
    return state_;
}

void Device::setState(Device::State state)
{
    std::unique_lock lock(lock_);
    setState(state, false);
}

const std::chrono::system_clock::time_point &Device::lastSeen() const
{
    std::shared_lock lock(lock_);
    return lastSeen_;
}

void Device::scanLoop()
{
    while (!exit_)
    {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_.wait(lk, [this] { return !queue_.empty(); });
        std::string uri = static_cast<std::string>(queue_.front());
        if (uri.empty())
        {
            LOG_ERROR(0, "Deque data is invalid!");
            continue;
        }
        LOG_DEBUG("scanLoop start for uri : %s",uri_.c_str());
        // let the plugin scan the device for media items
        auto plg = plugin();
        if (plg == nullptr)
        {
            LOG_ERROR(0, "plugin for %s is not invalid",uri.c_str());
            break;
        }
        setState(Device::State::Scanning);
        plg->scan(uri);
        setState(Device::State::Idle);
        if (processingDone())
            activateCleanUpTask();
        queue_.pop_front();
    }
}

bool Device::scan(IMediaItemObserver *observer)
{
    {
        std::shared_lock lock(lock_);
        if (!available_) {
            LOG_ERROR(0, "Device '%s' is not available", uri().c_str());
            return false;
        }
    }

    if (observer)
    {
        std::unique_lock lock(lock_);
        observer_ = observer;
    }

    LOG_INFO(0, "Plugin will scan '%s' for us", uri_.c_str());
    resetMediaItemCount();
    queue_.push_back(uri_);
    cv_.notify_one();
    return true;
}

IMediaItemObserver *Device::observer() const
{
    std::shared_lock lock(lock_);
    return observer_;
}

std::string Device::mountpoint() const
{
    std::shared_lock lock(lock_);
    return mountpoint_;
}

void Device::setMountpoint(const std::string &mp)
{
    std::unique_lock lock(lock_);
    mountpoint_ = mp;
}

std::shared_ptr<Plugin> Device::plugin() const
{
    PluginFactory fac;
    return fac.plugin(uri_);
}

void Device::incrementMediaItemCount(MediaItem::Type type)
{
    if (type == MediaItem::Type::EOL)
        return;

    std::unique_lock lock(lock_);

    auto cntIter = mediaItemCount_.find(type);
    if (cntIter == mediaItemCount_.end())
        mediaItemCount_[type] = 1;
    else
        mediaItemCount_[type]++;
    totalItemCount_++;
}

void Device::incrementProcessedItemCount(MediaItem::Type type)
{
    if (type == MediaItem::Type::EOL)
        return;

    std::unique_lock lock(lock_);

    auto cntIter = processedCount_.find(type);
    if (cntIter == processedCount_.end())
        processedCount_[type] = 1;
    else
        processedCount_[type]++;
    totalProcessedCount_++;
}

bool Device::processingDone()
{
    if (state_ == Device::State::Idle) {
        LOG_INFO(0, "Item Count : %d, Proccessed Count : %d", totalItemCount_, totalProcessedCount_);
        if (totalItemCount_ == totalProcessedCount_) {
            auto obs = observer();
            if (obs)
                obs->notifyDeviceScanned(this);
            return true;
        }
    }
    return false;
}

bool Device::addFileList(std::string &fpath)
{
    if (fpath.empty()) {
        LOG_ERROR(0, "Input fpath is invalid");
        return false;
    }
    fileList_.push_back(fpath);
    return true;
}

bool Device::isValidFile(std::string &fpath)
{
    if (fpath.empty()) {
        LOG_ERROR(0, "Input fpath is invalid");
        return false;
    }
    auto it = std::find(fileList_.begin(), fileList_.end(), fpath);
    if (it != fileList_.end()) {
        LOG_DEBUG("fpath %s is already exist", fpath.c_str());
        return false;
    }
    return true;
}

void Device::activateCleanUpTask()
{
    cleanUpTask_.sendMessage(this, nullptr);
}

void Device::resetMediaItemCount()
{
    mediaItemCount_.clear();
    processedCount_.clear();
    totalItemCount_ = totalProcessedCount_ = 0;
    fileList_.clear();
}

void Device::setState(Device::State state, bool force)
{
    if (state == state_)
        return;
    LOG_DEBUG("Device state change: %s -> %s",
        stateToString(state_).c_str(), stateToString(state).c_str());
    state_ = state;
}

int Device::mediaItemCount(MediaItem::Type type)
{
    std::shared_lock lock(lock_);

    auto cntIter = mediaItemCount_.find(type);
    if (cntIter == mediaItemCount_.end())
        return 0;
    else
        return mediaItemCount_[type];
}

bool Device::checkAlive()
{
    if (alive_ <= 0)
        return (alive_ != 0);

    if (--alive_ < 0)
        alive_ = -1;

    return !!(alive_);
}

void Device::resetAlive()
{
    alive_ = maxAlive_;
}
