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

#pragma once

#include <chrono>
#include <memory>
#include <map>
#include <mutex>

#if PERFCHECK_ENABLE
#define PERF_START(tag) \
    do { \
        PerfChecker *perf = PerfChecker::instance(); \
        perf->addToPerfMap(tag); \
        perf->start(tag); \
    } while(0);

#define PERF_END(tag) \
    do { \
        PerfChecker *perf = PerfChecker::instance(); \
        std::chrono::milliseconds elapsed = perf->end(tag); \
        LOG_PERF("[Elapsed Time][%s] %d ms", tag, (int)(elapsed.count())); \
    } while(0);
#else
#define PERF_START(tag)
#define PERF_END(tag)
#endif

class PerfTimeWatch
{
public:
    PerfTimeWatch()      {};
    ~PerfTimeWatch() {};
    void start() { _start   = std::chrono::system_clock::now(); };
    void end() { _end = std::chrono::system_clock::now(); };
    std::chrono::milliseconds elapsed() { return std::chrono::duration_cast<std::chrono::milliseconds>(_end - _start); };
private:
    std::chrono::time_point<std::chrono::system_clock> _start;
    std::chrono::time_point<std::chrono::system_clock> _end;
};

class PerfChecker
{
public:

    static PerfChecker *instance();

    virtual ~PerfChecker() {};

    bool addToPerfMap(std::string name);
    bool start(std::string   name);
    std::chrono::milliseconds end(std::string name);

protected:
    PerfChecker() {};

private:
    static std::unique_ptr<PerfChecker> instance_;
    std::map<std::string, PerfTimeWatch> perfMap_;
    std::mutex mutex_;
};

