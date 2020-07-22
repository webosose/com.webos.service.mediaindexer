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

#include "pdmlistener.h"
#include "ipdmobserver.h"

#include <algorithm>

const char *PdmListener::pdmUrl_ = "luna://com.webos.service.pdm/getAttachedAllDeviceList";
LSHandle *PdmListener::lsHandle_ = nullptr;
std::unique_ptr<PdmListener> PdmListener::instance_;

void PdmListener::init(LSHandle * lsHandle)
{
    lsHandle_ = lsHandle;
}

PdmListener *PdmListener::instance()
{
    if (!lsHandle_) {
        LOG_CRITICAL(0, "Luna bus handle not set");
        return nullptr;
    }

    if (!instance_.get())
        instance_.reset(new PdmListener);
    return instance_.get();
}

PdmListener::PdmListener() :
    deviceObservers_(),
    deviceMap_()
{
}

PdmListener::~PdmListener()
{
    // now delete the pdmDevice objects
    for (auto const &[path, dev] : deviceMap_)
        delete dev;
    deviceMap_.clear();
}

void PdmListener::setDeviceNotifications(IPdmObserver *observer,
    PdmDevice::DeviceType type, bool on)
{
    if (on) {
        // if we do not have any observers let's start subscription
        // now that we add one
        if (!deviceObservers_.empty()) {
            // Check if observer is already registered.
            auto it = std::find_if(deviceObservers_[type].begin(),
                deviceObservers_[type].end(),
                [observer](IPdmObserver *obs) { return (obs == observer); });

            if (it != deviceObservers_[type].end())
                return;
        } else {
            subscribe();
        }

        LOG_DEBUG("Add observer %p", observer);

        // Remember the observer for notifications.
        deviceObservers_[type].push_back(observer);

        // push already known devices to new observer
        for (auto const rootPath : deviceMapByType_[type]) {
            auto dev = deviceMap_[rootPath];
            observer->pdmUpdate(dev->dev(), true);
        }

    } else if (!deviceObservers_[type].empty()) {
        LOG_DEBUG("Remove observer %p", observer);
        deviceObservers_[type].remove(observer);
    }
}

void PdmListener::subscribe()
{
    auto subscription = pbnjson::Object();

    subscription.put("subscribe", true);
    LOG_INFO(0, "Subscribed for com.webos.service.pdm/getAttachedAllDeviceList");

    LSError lsError;
    LSErrorInit(&lsError);
    LSMessageToken sessionToken;

    if (!LSCall(lsHandle_, pdmUrl_, subscription.stringify().c_str(),
            PdmListener::onDeviceNotification, this, &sessionToken,
            &lsError)) {
        LOG_ERROR(0, "PDM service subscription error");
        return;
    }
}

void PdmListener::checkDevice(std::string &rootPath, pbnjson::JValue &dev)
{
    auto match = deviceMap_.find(rootPath);
    if (match != deviceMap_.end()) {
        deviceMap_[rootPath]->markDirty(false);
    } else {
        PdmDevice *pdmDev = new PdmDevice(dev);

        // do we support this type of device at all?
        if (pdmDev->type() == PdmDevice::DeviceType::UNSUPPORTED) {
            delete pdmDev;
            return;
        }

        // append newly detected device to list of available devices
        deviceMap_[rootPath] = pdmDev;

        // map this device to the device type
        deviceMapByType_[pdmDev->type()].push_back(rootPath);

        // tell all observers for this device type of the new device
        for (auto const obs : deviceObservers_[pdmDev->type()])
            obs->pdmUpdate(pdmDev->dev(), true);
    }
}

void PdmListener::cleanupDevices()
{
    for (auto const &[path, dev] : deviceMap_) {
        if (!dev->dirty())
            continue;

        // tell all observers for this device type of the removed
        // device
        for (auto const obs : deviceObservers_[dev->type()])
            obs->pdmUpdate(dev->dev(), false);

        // remove all references from the deviceMapByType_
        auto deviceType = dev->type();
        deviceMapByType_[deviceType].remove(dev->rootPath());

        // remove device from list of devices
        delete dev;
        deviceMap_.erase(path);
    }
}

bool PdmListener::onDeviceNotification(LSHandle *lsHandle, LSMessage *msg,
    void *ctx)
{
    PdmListener *pl = static_cast<PdmListener *>(ctx);

    const char *payload = LSMessageGetPayload(msg);
    // we do not need to check, the service implementation should do that
    pbnjson::JDomParser parser(pbnjson::JSchema::AllSchema());

    if (!parser.parse(payload)) {
        LOG_ERROR(0, "Invalid JSON message: %s", payload);
        return false;
    }

    LOG_INFO(0, "Pdm attached storage device update received: %s", payload);

    pbnjson::JValue domTree(parser.getDom());

    // check if all devices are gone
    if (!domTree.hasKey("deviceListInfo")) {
        pl->deviceMap_.clear();
        pl->deviceMapByType_.clear();
        return true;
    }

    auto deviceListInfo = domTree["deviceListInfo"];

    if (!deviceListInfo.isArray()) {
        LOG_ERROR(0, "deviceListInfo format is invalid");
        return false;
    }

    pbnjson::JValue storageDeviceList;
    for (ssize_t i = 0; i < deviceListInfo.arraySize(); ++i) {
        if (deviceListInfo[i].hasKey("storageDeviceList")) {
            storageDeviceList = deviceListInfo[i]["storageDeviceList"];
            break;
        }
    }

    // sanity check
    if (!storageDeviceList.isArray()) {
        LOG_DEBUG("storageDeviceList is not valid format");
        return true;
    }
    // our reference list is deviceMap_, mark all devices dirty and
    // then check if they really are
    for (auto const &[uri, dev] : pl->deviceMap_)
        dev->markDirty(true);

    // now iterate through the devices to find those where observers
    // are attached for
    for (ssize_t i = 0; i < storageDeviceList.arraySize(); ++i) {
        auto dev = storageDeviceList[i];
        auto rootPath = dev["rootPath"].asString();
        pl->checkDevice(rootPath, dev);
    }

    pl->cleanupDevices();
    return true;
}

