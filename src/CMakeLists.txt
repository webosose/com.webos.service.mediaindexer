# Copyright (c) 2019 LG Electronics, Inc.
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

# pmlogd
pkg_check_modules(PMLOG PmLogLib)
if (PMLOG_FOUND AND NOT STANDALONE)
  include_directories(${PMLOG_INCLUDE_DIRS})
  link_directories(${PMLOG_LIBRARY_DIRS})
  webos_add_compiler_flags(ALL ${PMLOG_CFLAGS_OTHER})
  link_libraries(${PMLOG_LIBRARIES})
  add_definitions(-DHAS_PMLOG)
endif ()

# glib
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB2_LIBRARY_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS})
link_libraries(${GLIB2_LIBRARIES})

# luna service
pkg_check_modules(LUNASERVICE2 luna-service2++)
if (LUNASERVICE2_FOUND AND NOT STANDALONE)
  include_directories(${LUNASERVICE2_INCLUDE_DIRS})
  link_directories(${LUNASERVICE2_LIBRARY_DIRS})
  webos_add_compiler_flags(ALL ${LUNASERVICE2_CFLAGS})
  link_libraries(${LUNASERVICE2_LIBRARIES})
  add_subdirectory(pdmlistener)
  list(APPEND LOCAL_LIBS pdmlistener)
  add_subdirectory(dbconnector)
  list(APPEND LOCAL_LIBS dbconnector)
  list(APPEND MODULES indexerservice.cpp)
  add_definitions(-DHAS_LUNA)
endif ()

# gstreamer
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
if (GSTREAMER_FOUND)
  include_directories(${GSTREAMER_INCLUDE_DIRS})
  link_directories(${GSTREAMER_LIBRARY_DIRS})
  webos_add_compiler_flags(ALL ${GSTREAMER_CFLAGS})
  link_libraries(${GSTREAMER_LIBRARIES})
  add_definitions(-DHAS_GSTREAMER)

  # contains GstDiscoverer
  pkg_check_modules(GSTPBUTILS REQUIRED gstreamer-pbutils-1.0)
  include_directories(${GSTPBUTILS_INCLUDE_DIRS})
  link_directories(${GSTPBUTILS_LIBRARY_DIRS})
  webos_add_compiler_flags(ALL ${GSTPBUTILS_CFLAGS})
  link_libraries(${GSTPBUTILS_LIBRARIES})
endif ()

# editline
if (STANDALONE)
  pkg_check_modules(LIBEDIT REQUIRED libedit>=3.0)
  include_directories(${LIBEDIT_INCLUDE_DIRS})
  link_directories(${LIBEDIT_LIBRARY_DIRS})
  webos_add_compiler_flags(ALL ${LIBEDIT_CFLAGS})
  link_libraries(${LIBEDIT_LIBRARIES})
  list(APPEND MODULES shell.cpp)
endif ()

# max. number of parallel running meta data extraction tasks
if (NOT PARALLEL_META_EXTRACTION)
  add_definitions(-DPARALLEL_META_EXTRACTION=10)
else ()
  add_definitions(-DPARALLEL_META_EXTRACTION=${PARALLEL_META_EXTRACTION})
endif ()

# build settings
add_executable(com.webos.service.mediaindexer
  ${MODULES}
  ${PLUGINS}
  main.cpp
  device.cpp
  mediaitem.cpp
  mediaparser.cpp
  mediaindexer.cpp)

include_directories(./)

# build and link the plugins
add_subdirectory(plugins)
# choose a meta data extractor
add_subdirectory(metadataextractors)

list(APPEND LOCAL_LIBS plugins metadataextractor pthread)
target_link_libraries(com.webos.service.mediaindexer ${LOCAL_LIBS})

webos_build_program(ADMIN)

install(TARGETS ${PROJECT_NAME} DESTINATION ${WEBOS_INSTALL_SBINDIR})
