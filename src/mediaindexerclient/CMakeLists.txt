# Copyright (c) 2020 LG Electronics, Inc.
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

include_directories(./)
include_directories(../)
include_directories(
                   ${CMAKE_CURRENT_SOURCE_DIR}
                   ${CMAKE_CURRENT_SOURCE_DIR}/connector
                   ${CMAKE_CURRENT_SOURCE_DIR}/utils
                   ${CMAKE_SOURCE_DIR}/src
                   ${CMAKE_SOURCE_DIR}/src/dbconnector
                   ${CMAKE_SOURCE_DIR}/src/log
                   )

set(MEDIA_INDEXER_CLIENT_HEADERS
    mediaindexer-common.h
    mediaindexerclient.h
    mediaindexerclient-api.h
   )

#set(MEDIA_INDEXER_CLIENT_SRC
#    mediaindexerclient.cpp
#    mediaindexerclient-api.cpp
#    lunaconnector.cpp
#   )
set(MEDIA_INDEXER_CLIENT_SRC
    mediaindexerclient.cpp
    mediaindexerclient-api.cpp
    connector/indexerconnector.cpp
    connector/mediadbconnector.cpp
    connector/connector.cpp
   )

set(MEDIA_INDEXER_CLIENT_LIB
    ${PMLOG_LIBRARIES}
    ${LUNASERVICE2_LIBRARIES}
    dbconnector
   )
add_library(mediaindexerclient SHARED ${MEDIA_INDEXER_CLIENT_SRC} ../log/logging.cpp)
target_link_libraries(mediaindexerclient
    ${MEDIA_INDEXER_CLIENT_LIB}
    ${PMLOG_LDFLAGS}
    )

install(TARGETS mediaindexerclient DESTINATION lib)
install(FILES ${MEDIA_INDEXER_CLIENT_HEADERS} DESTINATION include/mediaindexerclinet)
