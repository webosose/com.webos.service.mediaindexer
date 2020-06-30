# Copyright (c) 2019-2020 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

if("${WEBOS_TARGET_MACHINE}" STREQUAL "sa8155")
    message(STATUS "Using config file for webOS Auto sa8155")
    webos_build_configured_file(files/conf/sa8155/com.webos.service.mediaindexer.conf SYSCONFDIR com.webos.service.mediaindexer)
else()
    message(STATUS "Using config file for default MACHINE: " ${WEBOS_TARGET_MACHINE})
    webos_build_configured_file(files/conf/default/com.webos.service.mediaindexer.conf SYSCONFDIR com.webos.service.mediaindexer)
endif()
