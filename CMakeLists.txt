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

cmake_minimum_required(VERSION 2.8)

project(media-codec-interface)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

if(NOT DEFINED WEBOS_INSTALL_ROOT)
  set(WEBOS_INSTALL_ROOT /usr/local/webos/)
endif()

if(NOT DEFINED WEBOS_INSTALL_SBINDIR)
  set(WEBOS_INSTALL_SBINDIR /usr/sbin)
endif()

set(CMAKE_INSTALL_PREFIX ${WEBOS_INSTALL_ROOT}usr)

if (NOT DEFINED ENV{PKG_CONFIG_PATH})
  set(ENV{PKG_CONFIG_PATH} ${WEBOS_INSTALL_ROOT}usr/share/pkgconfig)
endif()

file(GLOB_RECURSE HEADERS "*.h*")
add_custom_target(show_headers SOURCES ${HEADERS})

find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTPLAYER gstreamer-player-1.0 REQUIRED)
pkg_check_modules(GSTREAMER gstreamer-1.0 REQUIRED)

set(GSTREAMER_INCLUDE_DIRS ${GSTPLAYER_INCLUDE_DIRS} ${GSTREAMER_INCLUDE_DIRS})
set(GSTREAMER_LIBRARIES ${GSTPLAYER_LIBRARIES} ${GSTREAMER_LIBRARIES})

include_directories(${GSTREAMER_INCLUDE_DIRS})
link_directories(${GSTREAMER_LIBRARIES})
option(WRAPPER_BUILD "Build C wrapper" OFF)
if(${WRAPPER_BUILD})
  include_directories(./resource/resource_wrapper)
  set(WRAPPER_SRC
      src/resource/resource_wrapper/stdlibcpp/buffer_encoder_lib.cpp
      src/resource/resource_wrapper/stdlibcpp/buffer_encoder_lib_impl.cpp
      src/resource/resource_wrapper/stdlibcpp/res_calculator.cpp
      src/resource/resource_wrapper/stdlibcpp/resource_manager_client.cpp
      src/resource/resource_wrapper/stdlibcpp/resource_manager_client_impl.cpp
  )
  set(WRAPPER_HEADERS
      src/resource/resource_wrapper/stdlibcpp/buffer_encoder_lib.h
      src/resource/resource_wrapper/stdlibcpp/res_calculator.h
      src/resource/resource_wrapper/stdlibcpp/resource_manager_client.h
  )

  set(WRAPPER_LIB media-codec-interface-wrapper)

  add_library(${WRAPPER_LIB} SHARED ${WRAPPER_SRC})

  set_target_properties (${WRAPPER_LIB} PROPERTIES VERSION 1.0 SOVERSION 1)

  pkg_check_modules(GST-VIDEO-ENCODER REQUIRED gst-video-encoder)
  include_directories(${GST-VIDEO-ENCODER_INCLUDE_DIRS})

  set(WRAPPER_LIBRARIES "-lresource_mgr_client -lmedia-resource-calculator -lgst-video-encoder")

  set_target_properties (${WRAPPER_LIB} PROPERTIES LINK_FLAGS "-Wl,-z,defs ${WRAPPER_LIBRARIES}")

  target_link_libraries(${WRAPPER_LIB})

  install(TARGETS ${WRAPPER_LIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(FILES ${WRAPPER_HEADERS} DESTINATION include)
else()
  add_subdirectory(src)
endif()

