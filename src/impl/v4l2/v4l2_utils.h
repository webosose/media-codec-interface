// Copyright (c) 2021 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SRC_IMPL_V4L2_V4L2_UTILS_H_
#define SRC_IMPL_V4L2_V4L2_UTILS_H_

#include <cerrno>
#include <fcntl.h>
#include <cstddef>
#include <cstdint>
#include <sys/ioctl.h>
#include <unistd.h>

#include <atomic>
#include <map>
#include <mutex>
#include <queue>
#include <set>

#include <linux/videodev2.h>

#include "base/decoder_types.h"
#include "base/encoder_types.h"
#include "base/fourcc.h"
#include "base/video_buffers.h"
#include "base/video_frame.h"

#define HANDLE_EINTR(x) ({ \
  decltype(x) eintr_wrapper_result; \
  do { \
    eintr_wrapper_result = (x); \
  } while (eintr_wrapper_result == -1 && errno == EINTR); \
  eintr_wrapper_result; \
})

#define IGNORE_EINTR(x) ({ \
  decltype(x) eintr_wrapper_result; \
  do { \
    eintr_wrapper_result = (x); \
    if (eintr_wrapper_result == -1 && errno == EINTR) { \
      eintr_wrapper_result = 0; \
    } \
  } while (0); \
  eintr_wrapper_result; \
})

#if defined(__clang__)
#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)  // no-op
#endif

#define GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

// GL_OES_EGL_image_external
#define GL_TEXTURE_EXTERNAL_OES 0x8D65

using V4L2BufferDestroyCb = std::function<void(enum v4l2_buf_type buffer_type)>;

#endif  // SRC_IMPL_V4L2_V4L2_UTILS_H_
