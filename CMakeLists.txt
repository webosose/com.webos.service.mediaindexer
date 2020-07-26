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

cmake_minimum_required(VERSION 2.8.7)

project(com.webos.service.mediaindexer C CXX)
set(CMAKE_BUILD_TYPE Debug)

include(webOS/webOS)
webos_modules_init(1 0 0 QUALIFIER RC7)
webos_component(1 0 0)

# general compiler setup
webos_add_compiler_flags(ALL -std=c++17 -Wall -Wextra
  -fno-permissive -pthread)
# disable specific warnings
webos_add_compiler_flags(ALL -Wno-unused-parameter
  -Wno-deprecated-declarations -Wno-type-limits -Wno-comment)
# promote specific warnings to errors
webos_add_compiler_flags(ALL -Werror=return-type -Werror=reorder
  -Werror=uninitialized)

# for pkg-config
find_package(PkgConfig)

# environment variable STANDALONE check
if (STANDALONE)
  message("Setup for standalone build")
  add_definitions(-DSTANDALONE)
else ()
  message("Setup for WebOS-Auto build")

  webos_build_system_bus_files()
  webos_build_configured_file(
    files/systemd/com.webos.service.mediaindexer.service
    SYSCONFDIR systemd/system)
endif ()

# linker options
webos_add_linker_options(ALL --no-undefined)

# get the sources
add_subdirectory(src)
add_subdirectory(test/mediaindexerclient)

# install configulation file
add_subdirectory(files/conf)

# doxygen section
find_package(Doxygen)
find_file(PLANTUML_JAR plantuml.jar
  PATHS /usr/share/plantuml /usr/share/* NO_DEFAULT_PATH)

if (DOXYGEN_FOUND)
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxconf.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doxconf)

  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

  # add 'doc' target to Makefile and show only errors or warnings
  add_custom_target(doc
    rm --force --recursive ${CMAKE_CURRENT_BINARY_DIR}/docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT} > /dev/null
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif ()
