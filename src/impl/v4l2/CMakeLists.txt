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

pkg_check_modules(LIBV4L2 QUIET libv4l2)
if (LIBV4L2_FOUND)
  include_directories(${LIBV4L2_INCLUDE_DIRS})
  link_directories(${LIBV4L2_LIBRARY_DIRS})
endif (LIBV4L2_FOUND)

list(APPEND MEDIA_IMPL_SRC
    impl/v4l2/v4l2_video_decoder.cpp
    impl/v4l2/generic_v4l2_device.cpp
    impl/v4l2/v4l2_buffers.cpp
    impl/v4l2/v4l2_device.cpp
    impl/v4l2/v4l2_queue.cpp
)

list(APPEND MEDIA_IMPL_LIBRARIES
    ${LIBV4L2_LIBRARIES}
)

if (${USE_ENCODER_BUILD} STREQUAL "V4L2")
add_definitions(-DUSE_V4L2_ENCODER)
list(APPEND MEDIA_IMPL_SRC
    impl/v4l2/v4l2_video_encoder.cpp
)
endif()