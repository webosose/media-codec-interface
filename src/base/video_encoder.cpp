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

#include "video_encoder.h"

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <cmath>
#include <cstring>
#include <sstream>
#include <map>
#include <memory>
#include <gio/gio.h>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/pbutils/pbutils.h>

#include "base/log.h"

namespace mcil {

namespace encoder {

VideoEncoder::VideoEncoder() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

VideoEncoder::~VideoEncoder() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
}

bool VideoEncoder::Initialize(const EncoderConfig* configData,
                              VideoEncoderDelegate* delegate) {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  delegate_ = delegate;
  return true;
}

bool VideoEncoder::Destroy() {
  MCIL_INFO_PRINT("%d %s", __LINE__, __FUNCTION__);
  return true;
}

bool VideoEncoder::EncodeBuffers(const uint8_t* yBuf, size_t ySize,
                                 const uint8_t* uBuf, size_t uSize,
                                 const uint8_t* vBuf, size_t vSize,
                                 uint64_t bufferTimestamp,
                                 const bool requestKeyFrame) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return false;
}

bool VideoEncoder::IsEncoderAvailable() {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return false;
}

bool VideoEncoder::UpdateEncodingParams(const EncodingParams* properties) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return true;
}

bool VideoEncoder::UpdateEncodingResolution(uint32_t width, uint32_t height) {
  MCIL_INFO_PRINT("%d %s : Error. Not Implemented", __LINE__, __FUNCTION__);
  return false;
}

}  // namespace encoder

}  // namespace mcil
