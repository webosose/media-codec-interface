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

#include "base/video_encoder.h"

#include <iostream>
#include <cstdlib>

#include "base/codec_types.h"
#include "gst/gst_video_encoder.h"
#include "v4l2/v4l2_video_encoder.h"

namespace mcil {

namespace {

enum CodecInstType {
  kCodecInstV4L2 = 0,
  kCodecInstGST = 1,
  kCodecInstMax = kCodecInstGST,
};

CodecInstType GetCodecInstToUse() {
  const char kCodecInstVariable[] = "USE_CODEC_INSTANCE";
  const std::string kCodecInstGstValue = "GST";
  const std::string kCodecInstV4L2Value = "V4L2";

  char *env_value = std::getenv(kCodecInstVariable);
  if (env_value != NULL) {
    if (std::string(env_value) == kCodecInstGstValue)
      return kCodecInstGST;
    else if (std::string(env_value) == kCodecInstV4L2Value)
      return kCodecInstV4L2;
  }
  return kCodecInstV4L2;
}

}  // namespace

// static
scoped_refptr<VideoEncoder> VideoEncoder::Create() {
  if (GetCodecInstToUse() == kCodecInstGST)
    return GstVideoEncoder::Create();

  return V4L2VideoEncoder::Create();
}

// static
SupportedProfiles VideoEncoder::GetSupportedProfiles() {
  if (GetCodecInstToUse() == kCodecInstGST)
    return GstVideoEncoder::GetSupportedProfiles();

  return V4L2VideoEncoder::GetSupportedProfiles();
}

}  // namespace mcil
