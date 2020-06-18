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

include_directories(./)
include_directories(../)

if (GSTREAMER_FOUND)
  list(APPEND EXTRACTORS gstreamerextractor.cpp)
endif ()

if (TAGLIB_FOUND)
  list(APPEND EXTRACTORS taglibextractor.cpp)
endif ()

pkg_check_modules(LIBPNG REQUIRED libpng)
if (LIBPNG_FOUND)
  include_directories(${LIBPNG_INCLUDE_DIRS})
  link_directories(${LIGPNG_LIBRARY_DIRS})
  link_libraries(${LIBPNG_LIBRARIES})
endif()

pkg_check_modules(gdk-pixbuf-2.0 REQUIRED gdk-pixbuf-2.0>=2.0)
if (gdk-pixbuf-2.0_FOUND)
  include_directories(${gdk-pixbuf-2.0_INCLUDE_DIRS})
  link_directories(${gdk-pixbuf-2.0_LIBRARY_DIRS})
  link_libraries(${gdk-pixbuf-2.0_LIBRARIES})
endif()

pkg_check_modules(libturbojpeg REQUIRED libturbojpeg)
if (libturbojpeg_FOUND)
  include_directories(${libturbojpeg_INCLUDE_DIRS})
  link_directories(${libturbojpeg_LIBRARY_DIRS})
  link_libraries(${libturbojpeg_LIBRARIES})
endif()

pkg_check_modules(libjpeg REQUIRED libjpeg)
if (libjpeg_FOUND)
  include_directories(${libjpeg_INCLUDE_DIRS})
  link_directories(${libjpeg_LIBRARY_DIRS})
  link_libraries(${libjpeg_LIBRARIES})
endif()

add_library(metadataextractor STATIC metadataextractor.cpp ${EXTRACTORS})
