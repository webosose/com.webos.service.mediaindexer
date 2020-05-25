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

class Device;

/// Interface definition for device observers.
class IDeviceObserver
{
public:
    virtual ~IDeviceObserver() {};

    /**
     * \brief Called if device becomes available/unavailable.
     *
     * The device is referenced from a std::shared_ptr as the device
     * might be destroyed in the plugin while still in use from the
     * observer.
     *
     * The callback is thread-safe.
     *
     * \param[in] device The device that changed availability.
     */
    virtual void deviceStateChanged(std::shared_ptr<Device> device) = 0;

    /**
     * \brief Called if device meta data has changed.
     *
     * The device is referenced from a std::shared_ptr as the device
     * might be destroyed in the plugin while still in use from the
     * observer.
     *
     * The callback is thread-safe.
     *
     * \param[in] device The device that changed availability.
     */
    virtual void deviceModified(std::shared_ptr<Device> device) = 0;

protected:
    IDeviceObserver() {};
};
