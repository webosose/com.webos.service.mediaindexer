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

#include "jsoncomposer.h"

template<>
pbnjson::JValue to_json(const mediaitem_t& item) {
    return pbnjson::JObject {{"uri",  item.uri},
                             {"hash", static_cast<int64_t>(item.hash)};

}

JSonComposer::JSonComposer() : _dom(pbnjson::JObject()) {}

std::string JSonComposer::result() {
    std::string result = _dom.stringify();
    return result;
}

