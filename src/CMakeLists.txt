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

pkg_check_modules(PMLOGLIB QUIET PmLogLib)
if (PMLOGLIB_FOUND)
  set(PMLOG_LIBRARIES PmLogLib)
endif (PMLOGLIB_FOUND)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(.)

include_directories(./base)

include_directories(./encoder)

add_subdirectory(encoder)

find_package(Threads REQUIRED)

set(MEDIA_CODEC_INTERFACE_LIB media-codec-interface)

set(MEDIA_CODEC_INTERFACE_HEADERS
    base/codec_types.h
    base/encoder_types.h
    base/types.h
    base/video_encoder.h
    video_encoder_api.h)

set(MEDIA_CODEC_INTERFACE_SRC
    base/log.cpp
    video_encoder_api.cpp
)

add_library(${MEDIA_CODEC_INTERFACE_LIB} SHARED ${MEDIA_CODEC_INTERFACE_SRC})
target_link_libraries(${MEDIA_CODEC_INTERFACE_LIB} video-encoder)

install(TARGETS ${MEDIA_CODEC_INTERFACE_LIB} DESTINATION lib)
install(FILES ${MEDIA_CODEC_INTERFACE_HEADERS} DESTINATION include/mcil)

#install media-codec-interface.pc file
install(FILES media-codec-interface.pc DESTINATION share/pkgconfig)

