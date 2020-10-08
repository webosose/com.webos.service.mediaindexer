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

message(STATUS "BUILDING test/luna")

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
                    )

set(SERVICE_TEST_NAME "com.webos.service.mediaindexerMock")
set(SERVICE_SRC_LIST IndexerService.cpp)

set(CLIENT_TEST_NAME "com.webos.service.mediaindexerMockClient")
set(CLIENT_SRC_LIST IndexerClient.cpp)

add_executable(${SERVICE_TEST_NAME} ${SERVICE_SRC_LIST} ${CMAKE_SOURCE_DIR}/src/log/logging.cpp)
add_executable(${CLIENT_TEST_NAME} ${CLIENT_SRC_LIST} ${CMAKE_SOURCE_DIR}/src/log/logging.cpp)
#confirming link language here avoids linker confusion and prevents errors seen previously
set_target_properties(${SERVICE_TEST_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${SERVICE_TEST_NAME}
                      ${GLIB2_LIBRARIES}
                      ${PMLOG_LIBRARIES}
                      ${LUNASERVICE2_LIBRARIES}
                      )

set_target_properties(${CLIENT_TEST_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${CLIENT_TEST_NAME}
                      ${GLIB2_LIBRARIES}
                      ${PMLOG_LIBRARIES}
                      ${LUNASERVICE2_LIBRARIES}
                      )

install(TARGETS ${SERVICE_TEST_NAME} ${CLIENT_TEST_NAME} DESTINATION sbin)

#com.webos.service.mediaindexerMock
configure_file(sysbus/com.webos.service.mediaindexermock.manifest.json.in sysbus/com.webos.service.mediaindexermock.manifest.json @ONLY)
configure_file(sysbus/com.webos.service.mediaindexermock.role.json.in sysbus/com.webos.service.mediaindexermock.role.json @ONLY)
configure_file(sysbus/com.webos.service.mediaindexermock.api.json.in sysbus/com.webos.service.mediaindexermock.api.json @ONLY)
configure_file(sysbus/com.webos.service.mediaindexermock.perm.json.in sysbus/com.webos.service.mediaindexermock.perm.json @ONLY)
configure_file(sysbus/com.webos.service.mediaindexermock.service.in sysbus/com.webos.service.mediaindexermock.service @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/com.webos.service.mediaindexermock.manifest.json
   DESTINATION share/luna-service2/manifests.d/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/com.webos.service.mediaindexermock.role.json
   DESTINATION share/luna-service2/roles.d/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/com.webos.service.mediaindexermock.api.json
   DESTINATION share/luna-service2/api-permissions.d/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/com.webos.service.mediaindexermock.perm.json
   DESTINATION share/luna-service2/client-permissions.d/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/com.webos.service.mediaindexermock.service
   DESTINATION share/luna-service2/services.d/)


#com.webos.service.mediaindexerMockClient
configure_file(sysbus/com.webos.service.mediaindexermockclient.manifest.json.in sysbus/com.webos.service.mediaindexermockclient.manifest.json @ONLY)
configure_file(sysbus/com.webos.service.mediaindexermockclient.role.json.in sysbus/com.webos.service.mediaindexermockclient.role.json @ONLY)
configure_file(sysbus/com.webos.service.mediaindexermockclient.perm.json.in sysbus/com.webos.service.mediaindexermockclient.perm.json @ONLY)
configure_file(sysbus/com.webos.service.mediaindexermockclient.service.in sysbus/com.webos.service.mediaindexermockclient.service @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/com.webos.service.mediaindexermockclient.manifest.json
   DESTINATION share/luna-service2/manifests.d/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/com.webos.service.mediaindexermockclient.role.json
   DESTINATION share/luna-service2/roles.d/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/com.webos.service.mediaindexermockclient.perm.json
   DESTINATION share/luna-service2/client-permissions.d/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sysbus/com.webos.service.mediaindexermockclient.service
   DESTINATION share/luna-service2/services.d/)

