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

include(FindPkgConfig)

pkg_check_modules(PMLOGLIB QUIET PmLogLib)
if (PMLOGLIB_FOUND)
  set(PMLOG_LIBRARIES PmLogLib)
endif (PMLOGLIB_FOUND)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(.)
include_directories(./api)
include_directories(./base)
include_directories(./impl)
include_directories(./resource)

set(MEDIA_CODEC_BASE_HEADERS
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
)

set(MEDIA_CODEC_BASE_SRC
    base/codec_types.cpp
    base/decoder_types.cpp
    base/encoder_types.cpp
    base/fourcc.cpp
    base/log.cpp
    base/thread.cpp
    base/video_buffers.cpp
    base/video_frame.cpp
)

set(MEDIA_CODEC_RESOURCE_SRC
    resource/requestor.cpp
    resource/video_resource.cpp
)

include(api/CMakeLists.txt)
include(impl/CMakeLists.txt)

#install media-codec-interface.pc file
configure_file(media-codec-interface.pc.in ${CMAKE_BINARY_DIR}/media-codec-interface.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/media-codec-interface.pc DESTINATION share/pkgconfig)
