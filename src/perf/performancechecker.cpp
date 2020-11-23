// Copyright (c) 2020 LG Electronics, Inc.
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

#include "performancechecker.h"
#include "logging.h"


std::unique_ptr<PerfChecker> PerfChecker::instance_;

PerfChecker *PerfChecker::instance()
{
    if (!instance_.get()) {
        instance_.reset(new PerfChecker);
    }
    return instance_.get();
}

bool PerfChecker::addToPerfMap(std::string name)
{    
    if (perfMap_.find(name) != perfMap_.end())
        return true;
    else
        perfMap_.emplace(std::string(name), PerfTimeWatch());
    return true;
}

bool PerfChecker::start(std::string name)
{
    std::unique_lock<std::mutex> lk(mutex_);
    auto timeWatch = perfMap_.find(name);
    if (timeWatch != perfMap_.end()) {
        timeWatch->second.start();
    } else {
        LOG_PERF("No Performance checker for %s", name.c_str());
        return false;
    }
    return true;
}

std::chrono::milliseconds PerfChecker::end(std::string name)
{
    std::unique_lock<std::mutex> lk(mutex_);
    auto timeWatch = perfMap_.find(name);
    if (timeWatch != perfMap_.end()) {
        timeWatch->second.end();
        return timeWatch->second.elapsed();
    } else {
        LOG_PERF("No Performance checker for %s", name.c_str());
        return std::chrono::milliseconds(0);
    }
}
