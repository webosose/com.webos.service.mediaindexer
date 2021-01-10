// Copyright (c) 2019-2021 LG Electronics, Inc.
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

#ifndef JSON_COMPOSER_H_
#define JSON_COMPOSER_H_

#include <type_traits>
#include <pbnjson.hpp>
#include <string>
#include "logging.h"


template<typename T>
pbnjson::JValue to_json(const T& value) {
    return pbnjson::JValue(value);
}

template<>
pbnjson::JValue to_json(const mediaitem_t &);

class JSonComposer {
public:
    JSonComposer();

    template<typename T>
    void put(const char* key, const T& value) {
        _dom.put(key, to_json(value));
    }

    template<typename T>
    void put(const std::string& key, const T& value) {
        put(key.c_str(), value);
    }

    template<typename T>
    void put(const T& value) {
        _dom = std::move(to_json(value));
    }

    std::string result();

private:
    pbnjson::JValue _dom;

};

#endif  // JSON_COMPOSER_H_

