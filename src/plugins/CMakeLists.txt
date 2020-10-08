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


include_directories(../log)

# libmtp
pkg_check_modules(LIBMTP libmtp>=1.0)
if (LIBMTP_FOUND AND STANDALONE)
  include_directories(${LIBMTP_INCLUDE_DIRS})
  link_libraries(${LIBMTP_LIBRARIES})
  # export compiler settings and environment to application
  add_definitions(-DHAS_LIBMTP)
  add_definitions(-DHAS_PLUGIN_MTP)
  list(APPEND PLUGINS mtp.cpp)
endif ()


# use com.webos.service.pdm if not in standalone mode
if (NOT STANDALONE)
  add_definitions(-DHAS_PDM)
  add_definitions(-DHAS_PLUGIN_MTP)
  list(APPEND PLUGINS mtp.cpp)
endif ()

# libupnp
pkg_check_modules(LIBUPNP libupnp>=1.6)
if (LIBUPNP_FOUND)
  include_directories(${LIBUPNP_INCLUDE_DIRS})
  link_libraries(${LIBUPNP_LIBRARIES})
  # export compiler settings and environment to application
  add_definitions(-DHAS_LIBUPNP)
  add_definitions(-DHAS_PLUGIN_UPNP)
  list(APPEND PLUGINS upnp.cpp)
endif ()

# a list of local storage pathes to observe, the list has the format
# <path>,<name>,<desc>;...
if (NOT STORAGE_DEVS)
  add_definitions(-DSTORAGE_DEVS="/media/multimedia,Media,Local Media Storage")
else ()
  add_definitions(-DSTORAGE_DEVS="${STORAGE_DEVS}")
endif ()

# gio
pkg_check_modules(GIO2 REQUIRED gio-2.0)
include_directories(${GIO2_INCLUDE_DIRS})
link_directories(${GIO2_LIBRARY_DIRS})
webos_add_compiler_flags(ALL ${GIO2_CFLAGS})
link_libraries(${GIO2_LIBRARIES})

# we need this for std::filesystem
link_libraries(stdc++fs)

# these do not have external dependencies and thus do not need checking
list(APPEND PLUGINS msc.cpp)
add_definitions(-DHAS_PLUGIN_MSC)
list(APPEND PLUGINS storage.cpp)
add_definitions(-DHAS_PLUGIN_STORAGE)
list(APPEND PLUGINS plugin.cpp)
list(APPEND PLUGINS pluginfactory.cpp)

add_library(plugins STATIC ${PLUGINS} ../log/logging.cpp)
