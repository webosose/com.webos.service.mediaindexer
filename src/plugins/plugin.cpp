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

#include "plugin.h"
#include "ideviceobserver.h"

#include <algorithm>
#include <filesystem>

#include <gio/gio.h>

namespace fs = std::filesystem;

bool Plugin::matchUri(const std::string &refUri, const std::string &testUri)
{
    return !testUri.compare(0, refUri.size(), refUri);
}

Plugin::~Plugin()
{
    // Nothing to be done here.
}

void Plugin::lock()
{
    lock_.lock_shared();
}

void Plugin::unlock()
{
    lock_.unlock_shared();
}

void Plugin::setDeviceNotifications(IDeviceObserver *observer, bool on)
{
    LOG_DEBUG("setDeviceNotifications Start");
    if (on) {
        bool firstObserver = addObserver(observer);

        {
            std::shared_lock lock(lock_);

            // now tell the new observer about the current state of
            // devices
            for (auto const dev :  devices_)
                notifyObserversStateChange(dev.second, observer);
        }

        // If this is the first observer we need to start device
        // detection.
        if (firstObserver) {
            LOG_INFO(0, "Enable device detection for: '%s'", uri_.c_str());
            runDeviceDetection(true);
        }
        else {
            LOG_INFO(0, "Not firstObserver, skip runDeviceDetection for: '%s'", uri_.c_str());
        }
    } else {
        LOG_DEBUG("removeObserver");
        removeObserver(observer);
    }
    LOG_DEBUG("setDeviceNotifications Done");
}

const std::string &Plugin::uri() const
{
    // no lock required, this is set only once from the constructor
    return uri_;
}

bool Plugin::injectDevice(std::shared_ptr<Device> device)
{
    bool isNew = false;

    if (!hasDevice(device->uri())) {
        std::unique_lock lock(lock_);
        devices_.emplace(device->uri(), device);
        isNew = true;
    }

    if (isNew)
        notifyObserversStateChange(device);

    return isNew;
}

bool Plugin::injectDevice(const std::string &uri, int alive, bool avail, std::string uuid)
{
    bool isNew = false;

    if (!hasDevice(uri)) {
        std::unique_lock lock(lock_);
        LOG_DEBUG("Make new device for uri : %s, uuid : %s", uri.c_str(), uuid.c_str());
        devices_[uri] = std::make_shared<Device>(uri, alive, avail, uuid);
        isNew = true;
    }

    if (isNew)
        notifyObserversStateChange(devices_[uri]);

    return isNew;
}

bool Plugin::addDevice(const std::string &uri, int alive)
{
    bool isNew = false;
    std::shared_ptr<Device> dev = nullptr;

    {
        std::unique_lock lock(lock_);
        dev = deviceUnlocked(uri);

        if (!dev) {
            dev = std::make_shared<Device>(uri, alive);
            LOG_DEBUG("Make new device for uri : %s", uri.c_str());
            devices_[uri] = dev;
            isNew = true;
        }
    }

    if (isNew) {
        notifyObserversStateChange(dev);
    } else {
        dev = device(uri);
        auto changed = dev->setAvailable(true);
        // now tell the observers if availability changed
        if (changed)
            notifyObserversStateChange(dev);
    }

    return isNew;
}

bool Plugin::addDevice(const std::string &uri, const std::string &mp, std::string uuid, int alive)
{
    bool isNew = false;
    std::shared_ptr<Device> dev = nullptr;

    {
        std::unique_lock lock(lock_);
        dev = deviceUnlocked(uri);

        if (!dev) {
            LOG_DEBUG("Make new device for uri : %s, uuid : %s", uri.c_str(), uuid.c_str());
            dev = std::make_shared<Device>(uri, alive, true, uuid);
            dev->setMountpoint(mp);
            devices_[uri] = dev;
            isNew = true;
        }
    }

    if (isNew) {
        notifyObserversStateChange(dev);
    } else {
        dev = device(uri);
        auto changed = dev->setAvailable(true);
        dev->setMountpoint(mp);
        dev->setUuid(uuid);
        // now tell the observers if availability changed
        if (changed)
            notifyObserversStateChange(dev);
    }

    return isNew;
}

bool Plugin::removeDevice(const std::string &uri)
{
    auto dev = device(uri);

    if (!dev)
        return false;

    auto changed = dev->setAvailable(false);

    // now tell the observers if availability changed
    if (changed)
        notifyObserversStateChange(dev);

    return changed;
}

void Plugin::removeAll(void)
{
    std::shared_lock lock(lock_);

    // mark the device as not available and notify observers if the
    // available state really changed
    for (auto const dev :  devices_) {
        if (dev.second->setAvailable(false))
            notifyObserversStateChange(dev.second);
    }
}

void Plugin::modifyDevice(const std::string &uri, Device::Meta type,
    const std::string &value)
{
    auto dev = device(uri);

    // notify observers if meta data has changed
    if (dev && dev->setMeta(type, value))
        notifyObserversModify(dev);
}

bool Plugin::hasDevice(const std::string &uri) const
{
    auto dev = device(uri);
    return !!dev;
}

std::shared_ptr<Device> Plugin::device(const std::string &uri) const
{
    std::shared_lock lock(lock_);

    return deviceUnlocked(uri);
}

void Plugin::checkDevices(void)
{
    std::shared_lock lock(lock_);

    for (auto const dev : devices_) {
        // if device is still alive step to next one
        auto alive = dev.second->available();
        if (dev.second->available(true) == alive)
            continue;
        notifyObserversStateChange(dev.second);
    }
}

const std::map<std::string, std::shared_ptr<Device>> &Plugin::devices() const
{
    return devices_;
}

bool Plugin::active() const
{
    // protect the request
    std::shared_lock lock(lock_);

    return !deviceObservers_.empty();
}

void Plugin::scan(const std::string &uri)
{
    LOG_DEBUG("scan start!");
    // first get the device from the uri
    auto dev = device(uri);
    if (!dev)
        return;

    auto obs = dev->observer();

    // get the device mountpoint
    auto mp = dev->mountpoint();
    if (mp.empty()) {
        LOG_ERROR(0, "Device '%s' has no mountpoint", dev->uri().c_str());
        return;
    }
    LOG_DEBUG("file scan start for mountpoint : %s!", mp.c_str());
    // do the file-tree-walk
    std::error_code err;
    try {
        for (auto &file : fs::recursive_directory_iterator(mp)) {
            if (!file.is_regular_file(err))
                continue;
            // get the file content type to decide if it can become a media
            // item
            gchar *contentType = NULL;
            gboolean uncertain;
            bool mimTypeSupported = false;
            contentType = g_content_type_guess(file.path().c_str(), NULL, 0,
                &uncertain);


            if (!contentType) {
                LOG_INFO(0, "MIME type detection is failed for '%s'",
                    file.path().c_str());
                // get the file extension for the ts or ps.
                std::string path = file.path();
                std::string ext = path.substr(path.find_last_of('.') + 1);

                if (!ext.compare("ts"))
                    contentType = "video/MP2T";
                else if (!ext.compare("ps"))
                    contentType = "video/MP2P";
                else {
                    LOG_INFO(0, "it's NOT ts/ps. need to check for '%s'", file.path().c_str());
                    continue;
                }
            }
            mimTypeSupported = MediaItem::mimeTypeSupported(contentType);

            if (uncertain && !mimTypeSupported) {
                LOG_INFO(0, "Invalid MIME type for '%s'", file.path().c_str());
                continue;
            }

            if (mimTypeSupported) {
                auto lastWrite = file.last_write_time();
                auto fileSize = file.file_size();
                auto hash = lastWrite.time_since_epoch().count();
                MediaItemPtr mi(new MediaItem(dev,
                        file.path(), contentType, hash, fileSize));
                obs->newMediaItem(std::move(mi));
            }

            g_free(contentType);
        }
    } catch (const std::exception &ex) {
        LOG_ERROR(0, "Exception caught while traversing through '%s', exception : %s",
            mp.c_str(), ex.what());
    }
    LOG_INFO(0, "File-tree-walk on device '%s' has been completed",
        dev->uri().c_str());
}

void Plugin::extractMeta(MediaItem &mediaItem, bool expand)
{
    LOG_ERROR(0, "No meta data extraction for '%s'", mediaItem.uri().c_str());
    // nothing to be done here, this should be implemented in the
    // derived plubin classes if needed
}

std::optional<std::string> Plugin::getPlaybackUri(const std::string &uri)
{
    auto dev = device(uri);
    if (!dev)
        return std::nullopt;

    if (!dev->available())
        return std::nullopt;

    auto path = std::string("file://");
    path.append(uri);
    // remove the device uri from the path
    path.erase(7, dev->uri().length());

    LOG_DEBUG("Playback uri for '%s' is '%s'", uri.c_str(), path.c_str());
    return path;
}

Plugin::Plugin(const std::string &uri) :
    uri_(uri),
    devices_(),
    deviceObservers_()
{
    // Nothing to be done here.
}

std::shared_ptr<Device> Plugin::deviceUnlocked(const std::string &uri) const
{
    for (auto &dev : devices_) {
        if (matchUri(dev.first, uri))
            return dev.second;
    }

    return nullptr;
}

bool Plugin::addObserver(IDeviceObserver *observer)
{
    std::unique_lock lock(lock_);

    if (!observer)
        return (deviceObservers_.size() == 1);

    // Check if observer is already registered.
    auto it = std::find_if(deviceObservers_.begin(), deviceObservers_.end(),
        [observer](IDeviceObserver *obs) { return (obs == observer); });
    if (it != deviceObservers_.end())
        return false;

    // Remember the observer for notifications.
    deviceObservers_.push_back(observer);

    return (deviceObservers_.size() == 1);
}

void Plugin::removeObserver(IDeviceObserver *observer)
{
    if (addObserver(nullptr)) {
        LOG_INFO(0, "Disable device detection for: '%s'", uri_.c_str());
        runDeviceDetection(false);
        removeAll();
    }

    std::unique_lock lock(lock_);
    deviceObservers_.remove(observer);
}

void Plugin::notifyObserversStateChange(const std::shared_ptr<Device> &device,
    IDeviceObserver *observer) const
{
    if (observer) {
        observer->deviceStateChanged(device);
    } else {
        std::shared_lock lock(lock_);
        for (auto const obs : deviceObservers_)
            obs->deviceStateChanged(device);
    }
}

void Plugin::notifyObserversModify(const std::shared_ptr<Device> &device) const
{
    std::shared_lock lock(lock_);
    for (auto const obs : deviceObservers_)
        obs->deviceModified(device);
}
