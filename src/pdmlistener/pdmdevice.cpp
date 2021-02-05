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

#include "pdmdevice.h"

PdmDevice::PdmDevice(const std::string& mountPath, pbnjson::JValue& dev)
    : type_(DeviceType::UNSUPPORTED)
    , mountPath_(mountPath)
    , dev_()
    , dirty_(false)
{
    dev_ = dev.duplicate();

    auto typeString = dev_["deviceType"].asString();

    if (typeString == "MTP")
        type_ = DeviceType::MTP;

    if (typeString == "USB_STORAGE")
        type_ = DeviceType::USB;
}

PdmDevice::~PdmDevice()
{
    // nothing to be done here
}

const std::string &PdmDevice::mountPath(void) const
{
    return mountPath_;
}

PdmDevice::DeviceType PdmDevice::type(void) const
{
    return type_;
}

const pbnjson::JValue &PdmDevice::dev(void) const
{
    return dev_;
}

void PdmDevice::markDirty(bool flag)
{
    dirty_ = flag;
}

bool PdmDevice::dirty(void) const
{
    return dirty_;
}
