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

#include "configurator.h"

std::unique_ptr<Configurator> Configurator::instance_;

Configurator *Configurator::instance()
{
    if (!instance_.get())
        instance_.reset(new Configurator(JSON_CONFIGURATION_FILE));
    return instance_.get();
}

Configurator::Configurator(std::string confPath)
    : confPath_(confPath)
    , force_sw_decoders_(false)
{
    init();
}

Configurator::~Configurator()
{
    extensions_.clear();
}

void Configurator::init()
{
    // get the JDOM tree from configuration file
    pbnjson::JValue root = pbnjson::JDomParser::fromFile(confPath_.c_str());
    if (!root.isObject()) {
        LOG_ERROR(0, "configuration file parsing error! need to check %s", confPath_.c_str());
        return;
    }

    if (!root.hasKey("force-sw-decoders"))
        LOG_WARNING(0, "GStreamer S/W decoder property is false! use H/W decoder!");

    force_sw_decoders_ = root["force-sw-decoders"].asBool();

    if (!root.hasKey("supportedMediaExtension")) {
        LOG_WARNING(0, "Can't find supportedMediaExtension field. need to check it!");
        return;
    }

    // get the extentions from supportedMediaExtension field
    pbnjson::JValue supportedExtensions = root["supportedMediaExtension"];

    // for audio extension
    if (supportedExtensions.hasKey("audio")) {
        pbnjson::JValue audioExtension = supportedExtensions["audio"];
        for (int idx = 0; idx < audioExtension.arraySize(); idx++)
            extensions_.insert(audioExtension[idx].asString());
    }

    // for video extension
    if (supportedExtensions.hasKey("video")) {
        pbnjson::JValue videoExtension = supportedExtensions["video"];
        for (int idx = 0; idx < videoExtension.arraySize(); idx++)
            extensions_.insert(videoExtension[idx].asString());
    }

    // for image extension
    if (supportedExtensions.hasKey("image")) {
        pbnjson::JValue imageExtension = supportedExtensions["image"];
        for (int idx = 0; idx < imageExtension.arraySize(); idx++)
            extensions_.insert(imageExtension[idx].asString());
    }

    printSupportedExtension();
}
  
bool Configurator::isSupportedExtension(std::string& ext) const
{
    printSupportedExtension();
    auto ret = extensions_.find(ext);
    if (ret != extensions_.end())
        return true;
    else
        return false;
}

std::set<std::string> Configurator::getSupportedExtensions() const
{
    return extensions_;
}

bool Configurator::getForceSWDecodersProperty() const
{
    return force_sw_decoders_; 
}

std::string Configurator::getConfigurationPath() const
{
    return confPath_;
}

bool Configurator::insertExtension(std::string& ext)
{
    std::pair<std::set<std::string>::iterator, bool> ret;
    ret = extensions_.insert(ext);
    return ret.second;
}

bool Configurator::removeExtension(std::string& ext)
{
    extensions_.erase(ext);
    return true;
}

void Configurator::printSupportedExtension() const
{
    LOG_DEBUG("--------------Supported extensions--------------");
    for (auto &ext : extensions_)
        LOG_DEBUG("%s", ext.c_str());
}
