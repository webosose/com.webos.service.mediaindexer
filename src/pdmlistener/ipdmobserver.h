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

/// Interface definition for pdm observers.
class IPdmObserver
{
public:
    virtual ~IPdmObserver() {};

    /**
     * \brief Called if com.webos.service.pdm has updates on the
     * registered device type. The device is pushed to the observer as
     * a JValue bacause we do not know which information is needed to
     * build the uri or such. In addition, a device may contain
     * several 'drives' whith different mount points. so a single pdm
     * device may turn up to contain several media indexer devices.
     *
     * \param[in] dev The device as JValue object.
     * \param[in] available If the device is available or not.
     */
    virtual void pdmUpdate(const pbnjson::JValue &dev, bool available) = 0;

protected:
    IPdmObserver() {};
};
