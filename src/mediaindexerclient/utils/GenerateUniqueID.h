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

#ifndef GENERATE_UNIQUE_ID_H_
#define GENERATE_UNIQUE_ID_H_

#include <string>
#include <random>
#include <functional>
#include <time.h>

namespace mediaindexerclient {
constexpr const int MEDIAINDEXER_UNIQUE_ID_LENGTH = 15;
class GenerateUniqueID {
    const std::string           source_;
    const int                   base_;
    const std::function<int()>  rand_;

public:
    explicit GenerateUniqueID(const std::string& src = "0123456789ABCDEFGIJKLMNOPQRSTUVWXYZabcdefgijklmnopqrstuvwxyz")
        : source_(src)
        , base_(source_.size())
        , rand_(std::bind(std::uniform_int_distribution<int>(0, base_ - 1),
                          std::mt19937(std::random_device()()))) {}

    std::string operator ()()
    {
        struct timespec time;
        std::string s(MEDIAINDEXER_UNIQUE_ID_LENGTH, '0');

	    clock_gettime(CLOCK_MONOTONIC, &time);

    	s[0] = '_'; // Prepend uid with _ to comply with luna requirements
        for (int i = 1; i < MEDIAINDEXER_UNIQUE_ID_LENGTH; ++i) {
            if (i < 5 && i < MEDIAINDEXER_UNIQUE_ID_LENGTH - 6) {
                int index = time.tv_nsec % base_;
                if (index >= 0 && index < base_) {
                    s[i] = source_[index];
                }
                time.tv_nsec /= base_;
            } else if (time.tv_sec > 0 && i < MEDIAINDEXER_UNIQUE_ID_LENGTH - 3) {
                int index = time.tv_sec % base_;
                if (index >= 0 && index < base_) {
                    s[i] = source_[index];
                }
                time.tv_sec /= base_;
            } else {
                int index = rand_();
                if (index >= 0 && index < base_) {
                    s[i] = source_[index];
                }
            }
        }
        return s;
    }
};

} // namespace mediaindexerclient

#endif /* GENERATE_UNIQUE_ID_H_ */
