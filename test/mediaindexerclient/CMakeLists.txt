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

message(STATUS "BUILDING test/mediaindexerclient")

# glib
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB2_LIBRARY_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS})
link_libraries(${GLIB2_LIBRARIES})

pkg_check_modules(PMLOG PmLogLib)
include_directories(${PMLOG_INCLUDE_DIRS})
link_directories(${PMLOG_LIBRARY_DIRS})
webos_add_compiler_flags(ALL ${PMLOG_CFLAGS_OTHER})
link_libraries(${PMLOG_LIBRARIES})
add_definitions(-DHAS_PMLOG)

pkg_check_modules(LUNASERVICE2 luna-service2++)
include_directories(${LUNASERVICE2_INCLUDE_DIRS})
link_directories(${LUNASERVICE2_LIBRARY_DIRS})
webos_add_compiler_flags(ALL ${LUNASERVICE2_CFLAGS})
link_libraries(${LUNASERVICE2_LIBRARIES})
add_definitions(-DHAS_LUNA)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/src/
                    ${CMAKE_SOURCE_DIR}/src/mediaindexerclient
                    ${CMAKE_SOURCE_DIR}/src/mediaindexerclient/connector
                    ${CMAKE_SOURCE_DIR}/src/mediaindexerclient/utils
                    ${CMAKE_SOURCE_DIR}/src/dbconnector
                    ${CMAKE_SOURCE_DIR}/src/log
                    )

set(TESTNAME "mediaindexerclient_test")
set(SRC_LIST MediaIndexerClientTest.cpp)

set(API_TEST_NAME "mediaindexerclient_apitest")
set(API_TEST_SRC_LIST IndexerClientAPITest.cpp)

add_executable (${TESTNAME} ${SRC_LIST} ${CMAKE_SOURCE_DIR}/src/log/logging.cpp)
add_executable (${API_TEST_NAME} ${API_TEST_SRC_LIST} ${CMAKE_SOURCE_DIR}/src/log/logging.cpp)
#confirming link language here avoids linker confusion and prevents errors seen previously
set_target_properties(${TESTNAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${API_TEST_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${TESTNAME}
                      ${GLIB2_LIBRARIES}
                      ${PMLOG_LIBRARIES}
                      ${LUNASERVICE2_LIBRARIES}
                      mediaindexerclient
                      )

target_link_libraries(${API_TEST_NAME}
                      ${GLIB2_LIBRARIES}
                      ${PMLOG_LIBRARIES}
                      ${LUNASERVICE2_LIBRARIES}
                      mediaindexerclient
                      )

install(TARGETS ${TESTNAME} ${API_TEST_NAME} DESTINATION sbin)

configure_file(sysbus/mediaindexerclient_test.manifest.json.in sysbus/mediaindexerclient_test.manifest.json @ONLY)
configure_file(sysbus/mediaindexerclient_test.role.json.in sysbus/mediaindexerclient_test.role.json @ONLY)
configure_file(sysbus/mediaindexerclient_apitest.manifest.json.in sysbus/mediaindexerclient_apitest.manifest.json @ONLY)
configure_file(sysbus/mediaindexerclient_apitest.role.json.in sysbus/mediaindexerclient_apitest.role.json @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/mediaindexerclient_test.manifest.json
   DESTINATION share/luna-service2/manifests.d/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/mediaindexerclient_test.role.json
   DESTINATION share/luna-service2/roles.d/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/mediaindexerclient_apitest.manifest.json
   DESTINATION share/luna-service2/manifests.d/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/mediaindexerclient_apitest.role.json
   DESTINATION share/luna-service2/roles.d/)
