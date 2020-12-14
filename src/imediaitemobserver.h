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

#include <memory>

class MediaItem;
class Device;

typedef std::shared_ptr<Device> DevicePtr;

/// Interface definition for mediaitem observers.
class IMediaItemObserver
{
public:
    virtual ~IMediaItemObserver() {};

    /**
     * \brief Called if mediaitem detects a new media item or if the
     * media meta data has been updated (in which dase the parsed flag
     * should be set on the media item).
     *
     * The callback is thread-safe.
     *
     * \param[in] mediaItem The media item.
     */
    virtual void newMediaItem(std::unique_ptr<MediaItem> mediaItem) = 0;

    /**
     * \brief Called if mediaitem needs updated meta data.
     *
     * The callback is thread-safe.
     *
     * \param[in] mediaItem The media item.
     */
    virtual void metaDataUpdateRequired(std::unique_ptr<MediaItem>  mediaItem) = 0;

    /**
     * \brief Called if device cleanup can be started.
     *
     * The callback is thread-safe.
     *
     * \param[in] dev The device.
     */
    virtual void cleanupDevice(Device* dev) = 0;

    /**
     * \brief Called when dirty flag handling remains on the device.
     *
     * \param[in] dev The device.
     */
    virtual void flushUnflagDirty(Device* dev) = 0;

     /**
     * \brief notify after specified device has been scanned.
     *
     *
     * \param[in] dev The device.
     */
    virtual void notifyDeviceScanned(Device* dev) = 0;

protected:
    IMediaItemObserver() {};
};
