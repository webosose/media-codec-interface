# Copyright (c) 2021 LG Electronics, Inc.
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

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB2_LIBRARY_DIRS})

option(ENABLE_WRAPPER "Enable C wrapper" OFF)
option(WEBOS_CLANG_BUILD "Build with clang" OFF)
if(${WEBOS_CLANG_BUILD} AND NOT ${ENABLE_WRAPPER})
  pkg_check_modules(PBNJSON pbnjson_cpp_clang REQUIRED)
else()
  pkg_check_modules(PBNJSON pbnjson_cpp REQUIRED)
endif()
include_directories(${PBNJSON_INCLUDE_DIRS})
link_directories(${PBNJSON_LIBRARY_DIRS})

find_package(Boost REQUIRED COMPONENTS regex)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

if(${WEBOS_CLANG_BUILD})
  set(MEDIA_IMPL_LIB media-codec-impl-clang)
else()
  set(MEDIA_IMPL_LIB media-codec-impl)
endif()

set(MEDIA_IMPL_SRC
  base/log.cpp
  impl/video_decoder.cpp
  impl/video_encoder.cpp
)

set(MEDIA_IMPL_LIBRARIES
    ${CMAKE_THREAD_LIBS_INIT}
    ${GLIB2_LIBRARIES}
    ${PBNJSON_LIBRARIES}
    ${PMLOG_LIBRARIES}
)

include(impl/gst/CMakeLists.txt)
include(impl/v4l2/CMakeLists.txt)

get_filename_component(DIR_PATH "${_dir}" REALPATH)
if(EXISTS "${DIR_PATH}/impl/ext" AND EXISTS "${DIR_PATH}/impl/ext/CMakeLists.txt")
  include(impl/ext/CMakeLists.txt)
endif()

add_library(${MEDIA_IMPL_LIB} SHARED ${MEDIA_IMPL_SRC})
set_target_properties (${MEDIA_IMPL_LIB} PROPERTIES VERSION 1.0 SOVERSION 1)
target_link_libraries(${MEDIA_IMPL_LIB} ${MEDIA_IMPL_LIBRARIES})

if(${ENABLE_WRAPPER})
  set_target_properties (${MEDIA_IMPL_LIB} PROPERTIES LINK_FLAGS "-lmedia-codec-interface-wrapper")
endif()

install(TARGETS ${MEDIA_IMPL_LIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
