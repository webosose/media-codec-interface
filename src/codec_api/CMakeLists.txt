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

find_package(Threads REQUIRED)

pkg_check_modules(RESOURCE-CALCULATOR REQUIRED media-resource-calculator)
include_directories(${RESOURCE-CALCULATOR_INCLUDE_DIRS})

set(MEDIA_CODEC_INTERFACE_LIB media-codec-interface)

set(MEDIA_CODEC_INTERFACE_HEADERS
    ${MEDIA_CODEC_BASE_HEADERS}
    codec_api/video_decoder_api.h
    codec_api/video_encoder_api.h)

set(MEDIA_CODEC_INTERFACE_SRC
    ${MEDIA_CODEC_BASE_SRC}
    ${MEDIA_CODEC_RESOURCE_SRC}
    codec_api/video_decoder_api.cpp
    codec_api/video_encoder_api.cpp
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

