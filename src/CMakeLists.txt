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

include(webOS/webOS)
webos_modules_init(1 6 3)

find_package(PkgConfig REQUIRED)

pkg_check_modules(PMLOGLIB QUIET PmLogLib)
if (PMLOGLIB_FOUND)
  set(PMLOG_LIBRARIES PmLogLib)
endif (PMLOGLIB_FOUND)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(.)

include_directories(./base)

include_directories(./codec_impl)

add_subdirectory(codec_impl)

pkg_check_modules(RESOURCE-CALCULATOR REQUIRED media-resource-calculator)
include_directories(${RESOURCE-CALCULATOR_INCLUDE_DIRS})

find_package(Threads REQUIRED)

set(MEDIA_CODEC_INTERFACE_LIB media-codec-interface)

set(MEDIA_CODEC_INTERFACE_HEADERS
    base/codec_types.h
    base/decoder_types.h
    base/encoder_types.h
    base/fourcc.h
    base/optional.h
    base/ref_counted.h
    base/scoped_refptr.h
    base/thread.h
    base/video_buffers.h
    base/video_decoder.h
    base/video_decoder_client.h
    base/video_encoder_client.h
    base/video_encoder.h
    base/video_frame.h
    resource/video_resource.h
    video_decoder_api.h
    video_encoder_api.h)

set(MEDIA_CODEC_INTERFACE_SRC
    base/codec_types.cpp
    base/decoder_types.cpp
    base/encoder_types.cpp
    base/fourcc.cpp
    base/log.cpp
    base/thread.cpp
    base/video_buffers.cpp
    base/video_decoder.cpp
    base/video_encoder.cpp
    base/video_frame.cpp
    resource/requestor.cpp
    resource/video_resource.cpp
    video_decoder_api.cpp
    video_encoder_api.cpp
)

set(MEDIA_CODEC_INTERFACE_LIBRARIES
    ${RESOURCE-CALCULATOR_LDFLAGS}
    resource_mgr_client
    resource_mgr_client_c
)

add_library(${MEDIA_CODEC_INTERFACE_LIB} SHARED ${MEDIA_CODEC_INTERFACE_SRC})
target_link_libraries(${MEDIA_CODEC_INTERFACE_LIB} ${MEDIA_CODEC_INTERFACE_LIBRARIES} media-codec-impl)

install(TARGETS ${MEDIA_CODEC_INTERFACE_LIB} DESTINATION lib)
install(FILES ${MEDIA_CODEC_INTERFACE_HEADERS} DESTINATION include/mcil)

#install media-codec-interface.pc file
configure_file(media-codec-interface.pc.in ${CMAKE_BINARY_DIR}/media-codec-interface.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/media-codec-interface.pc DESTINATION share/pkgconfig)

