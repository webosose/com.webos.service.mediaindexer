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

#include "mediaitem.h"
#include <pbnjson.hpp>
#include <set>

/// Configurator class for media indexer configuration from json conf file.
class Configurator
{
public:
    static Configurator *instance();
    
    virtual ~Configurator();

    void init();
    bool isSupportedExtension(std::string& ext) const;
    std::set<std::string> getSupportedExtensions() const;
    bool getForceSWDecodersProperty() const;
    std::string getConfigurationPath() const;
    bool insertExtension(std::string& ext);
    bool removeExtension(std::string& ext);
    void printSupportedExtension() const; 

 private:
    /// Get message id.
    LOG_MSGID;

    /// Singleton
    Configurator(std::string confPath);

    /// supported extensions
    std::set<std::string> extensions_;

    /// configuration file path
    std::string confPath_;

    /// GStreamer property for software decoding
    bool force_sw_decoders_;


    /// Singleton instance object.
    static std::unique_ptr<Configurator> instance_;
};
