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

find_package(PkgConfig REQUIRED)

option(ENABLE_WRAPPER "Enable C wrapper" OFF)
option(WEBOS_CLANG_BUILD "Build with clang" OFF)
if(${WEBOS_CLANG_BUILD} AND NOT ${ENABLE_WRAPPER})
  pkg_check_modules(PBNJSON pbnjson_cpp_clang REQUIRED)
else()
  pkg_check_modules(PBNJSON pbnjson_cpp REQUIRED)
endif()
include_directories(${PBNJSON_INCLUDE_DIRS})
link_directories(${PBNJSON_LIBRARY_DIRS})

option(WEBOS_CLANG_BUILD "Build with clang" OFF)
if(NOT ${ENABLE_WRAPPER})
  if(${WEBOS_CLANG_BUILD})
    pkg_check_modules(GST-VIDEO-ENCODER REQUIRED gst-video-encoder-clang)
  else()
    pkg_check_modules(GST-VIDEO-ENCODER REQUIRED gst-video-encoder)
  endif()
  include_directories(${GST-VIDEO-ENCODER_INCLUDE_DIRS})

  list(APPEND MEDIA_IMPL_LIBRARIES
      ${GST-VIDEO-ENCODER_LIBRARIES}
  )
endif()

list(APPEND MEDIA_IMPL_SRC
    impl/gst/gst_video_encoder.cpp
)

if(${ENABLE_WRAPPER})
  list(APPEND MEDIA_IMPL_SRC
      impl/gst/gst_wrapper/buffer_encoder_wrapper.cpp
  )

  add_definitions(-DENABLE_WRAPPER)
endif()
