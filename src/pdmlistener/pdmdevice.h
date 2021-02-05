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

#include <pbnjson.hpp>

/// Listener for com.webos.service.pdm.
class PdmDevice
{
public:
    /// Device type for updates.
    enum class DeviceType {
        UNSUPPORTED,
        USB,
        MTP,
    };

    /**
     * \brief Constructs a new device from dom tree.
     *
     * \param[in] dev The JValue from pdm luna notification.
     */
    PdmDevice(const std::string &mountPath, pbnjson::JValue &dev);
    virtual ~PdmDevice();

    /// Give us the device mountPath property.
    const std::string &mountPath(void) const;
    /// Give us the device type classifier.
    DeviceType type(void) const;
    /// Give us the device JSON description.
    const pbnjson::JValue &dev(void) const;

    /**
     * \brief Mark the device dirty.
     *
     * This is used to delete devices that are no longer advertised
     * from the pdm service.
     *
     * \param[in] flag Dirty flag.
     */
    void markDirty(bool flag);
    /// Give us the device dirty flag.
    bool dirty(void) const;

private:
    /// Construction needs dom tree.
    PdmDevice();

    /// Device type.
    DeviceType type_;
    /// Mount path of device.
    std::string mountPath_;
    /// Parsed JSON message for device.
    pbnjson::JValue dev_;
    /// Indicates that device has not yet been marked available.
    bool dirty_;
};
