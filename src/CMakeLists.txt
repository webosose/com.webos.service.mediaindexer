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

# taglib
pkg_check_modules(TAGLIB REQUIRED taglib)
if (TAGLIB_FOUND)
  include_directories(${TAGLIB_INCLUDE_DIRS})
  link_directories(${TAGLIB_LIBRARY_DIRS})
  webos_add_compiler_flags(ALL ${TAGLIB_CFLAGS})
  link_libraries(${TAGLIB_LIBRARIES})
  add_definitions(-DHAS_TAGLIB)
endif ()

# pbnjson
pkg_check_modules(LIBPBNJSON REQUIRED pbnjson_cpp)
webos_add_compiler_flags(ALL ${LIBPBNJSON_CFLAGS})

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

# add definition for the JSON configuration file
if ("${WEBOS_TARGET_MACHINE}" STREQUAL "sa8155")
  message(STATUS "Add definition for JSON configuration file for webOS Auto sa8155")
  add_definitions(-DJSON_CONFIGURATION_FILE="/etc/com.webos.service.mediaindexer/com.webos.service.mediaindexer.conf")
else ()
  message(STATUS "Add definition for JSON configuration file for default machine")
  add_definitions(-DJSON_CONFIGURATION_FILE="/etc/com.webos.service.mediaindexer/com.webos.service.mediaindexer.conf")
endif ()

# build settings
add_executable(com.webos.service.mediaindexer
  ${MODULES}
  ${PLUGINS}
  main.cpp
  device.cpp
  mediaitem.cpp
  mediaparser.cpp
  dbobserver.cpp
  localeobserver.cpp
  task.cpp
  mediaindexer.cpp
  indexerserviceclientsmgrimpl.cpp)

include_directories(./)
include_directories(./jsonparser)
include_directories(./log)
include_directories(./mediaindexerclient)
include_directories(./perf)

# definitions for thumbnail directory
add_definitions(-DTHUMBNAIL_DIRECTORY="/media/.thumbnail/")
add_definitions(-DPERFCHECK_ENABLE=1)

# build and link the plugins
add_subdirectory(plugins)
# choose a meta data extractor
add_subdirectory(metadataextractors)
add_subdirectory(mediaindexerclient)
add_subdirectory(perf)

list(APPEND LOCAL_LIBS plugins metadataextractor pthread perf)
target_link_libraries(com.webos.service.mediaindexer ${LOCAL_LIBS})

webos_build_program(ADMIN)

install(TARGETS ${PROJECT_NAME} DESTINATION ${WEBOS_INSTALL_SBINDIR})
