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

#include "msc.h"
#if defined HAS_PDM
#include "pdmlistener/pdmlistener.h"
#endif

#include <string>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <system_error>

std::string Usb::uri("msc");
std::shared_ptr<Plugin> Usb::instance_ = nullptr;

std::shared_ptr<Plugin> Usb::instance()
{
    if (!Usb::instance_)
        Usb::instance_ = std::make_shared<Usb>();
    return Usb::instance_;
}

Usb::Usb() :
    Plugin(Usb::uri)
{
    // nothing to be done here
}

Usb::~Usb()
{
    runDeviceDetection(false);
}

#if defined HAS_PDM
void Usb::pdmUpdate(const pbnjson::JValue &dev, bool available)
{
    // sanity check
    if (!dev.hasKey("storageDriveList"))
        return;

    auto drives = dev["storageDriveList"];

    // sanity check
    if (drives.arraySize() == 0)
        return;

    for (ssize_t i = 0; i < drives.arraySize(); ++i) {
        auto drive = drives[i];

        // sanity check
        if (!drive.hasKey("mountName"))
            continue;
        if (!drive.hasKey("uuid"))
            continue;

        // remember mountpoint for device creation
        auto mp = drive["mountName"].asString();
        auto uuid = drive["uuid"].asString(); // Device UUID.
        std::stringstream suri;
        suri << Usb::uri << "://" << // Uri identifier.
            uuid; // Device UUID.

        auto uri = suri.str();
        if (available) {
            addDevice(uri, mp, std::move(uuid));
            // add meta data, for USB case we may only know the volume
            // label
            std::string label;
            if (drive.hasKey("volumeLabel")) {
                label = drive["volumeLabel"].asString();
                // replace escaped spaces with spaces
                while(label.find(space_) != std::string::npos)
                    label.replace(label.find(space_),
                        space_.size(), " ");
                modifyDevice(uri, Device::Meta::Name, label);
            }
            // the description can be build from vendor + device name
            if (dev.hasKey("productName")) {
                auto desc = dev["productName"].asString();
                // use device name for name if no volume label has
                // been set
                if (label.empty())
                    modifyDevice(uri, Device::Meta::Name, desc);
                modifyDevice(uri, Device::Meta::Description, desc);
            }
        } else {
            removeDevice(uri);
        }
    }
}
#endif

int Usb::runDeviceDetection(bool start)
{
#if defined HAS_PDM
    auto listener = PdmListener::instance();
    if(listener)
      listener->setDeviceNotifications(
        static_cast<IPdmObserver *>(this), PdmDevice::DeviceType::USB, start);
#else
    // there is no device detection for USB mass storage devices
    // implemented, this is so similar to what the storage plugin does
    // that it is not really worth the effort
#endif

    return 0;
}
