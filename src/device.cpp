// Copyright (c) 2019 LG Electronics, Inc.
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

Device::Device(const std::string &uri, int alive, bool avail) :
    uri_(uri),
    mountpoint_(""),
    lastSeen_(),
    available_(avail),
    alive_(alive),
    maxAlive_(alive)
{
    lastSeen_ = std::chrono::system_clock::now();
}

Device::~Device()
{
    // nothing to be done here, the only allocated object is the
    // scanner thread which runs in detached mode and thus does not
    // need any cleanup
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
    }

    auto changed = (before != avail);
    if (avail && changed)
        lastSeen_ = std::chrono::system_clock::now();

    return changed;
}

const std::string &Device::uri() const
{
    return uri_;
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

const std::chrono::system_clock::time_point &Device::lastSeen() const
{
    std::shared_lock lock(lock_);
    return lastSeen_;
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

    {
        std::unique_lock lock(lock_);
        observer_ = observer;
    }

    // let the plugin scan the device for media items
    auto plg = plugin();
    LOG_INFO(0, "Plugin will scan '%s' for us", uri_.c_str());
    std::thread t(&Plugin::scan, plg, std::ref(uri_));
    // this is a worker thread, in case the device becomes unavailable
    // the thread will terminate due to some access errors anyway
    t.detach();

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
    std::unique_lock lock(lock_);

    auto cntIter = mediaItemCount_.find(type);
    if (cntIter == mediaItemCount_.end())
        mediaItemCount_[type] = 1;
    else
        mediaItemCount_[type]++;
}

void Device::resetMediaItemCount()
{
    mediaItemCount_.clear();
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
