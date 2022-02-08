// Copyright (c) 2022 LG Electronics, Inc.
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

#include "video_resource.h"

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <cmath>
#include <cstring>
#include <sstream>
#include <map>

#include "base/log.h"
#include "decoder_types.h"

#define MCIL_MAX_FRAME_RATE 60 //Set to maximum framerate supported in webcodec

#ifdef MCIL_DEBUG_PRINT
//#undef MCIL_DEBUG_PRINT
#endif
//#define MCIL_DEBUG_PRINT MCIL_INFO_PRINT

namespace mcil {

namespace decoder {

//Static member definition
VideoResource& VideoResource::GetInstance() {
  static VideoResource rm_instance;
  return rm_instance;
}

VideoResource::VideoResource()
  : requestor_(std::make_unique<ResourceRequestor> ()) {
}

VideoResource::~VideoResource() {
}

bool VideoResource::Acquire(DeviceType device_type,
                            VideoCodecType video_codec,
                            uint32_t frame_width,
                            uint32_t frame_height,
                            std::string& resources,
                            int32_t *vdec_index) {
  MCIL_INFO_PRINT(": called");

  PortResource_t resourceMMap;
  ACQUIRE_RESOURCE_INFO_T resource_info;

  video_info_t video_stream_info = {};

  video_stream_info.width = frame_width;
  video_stream_info.height = frame_height;

  if (device_type == V4L2_DECODER) {
    MCIL_DEBUG_PRINT(": decoder resource needed");
    video_stream_info.encode = VIDEO_CODEC_NONE;
    video_stream_info.decode = video_codec;
  } else if (device_type == V4L2_ENCODER) {
    MCIL_DEBUG_PRINT(": encoder resource needed");
    video_stream_info.encode = video_codec;
    video_stream_info.decode = VIDEO_CODEC_NONE;
  } else {
    return false;
  }

  video_stream_info.frame_rate.num = MCIL_MAX_FRAME_RATE;
  video_stream_info.frame_rate.den = 1;
  MCIL_DEBUG_PRINT("[video info] width: %d, height: %d, frameRate: %d/%d",
            video_stream_info.width, video_stream_info.height,
            video_stream_info.frame_rate.num, video_stream_info.frame_rate.den);

  program_info_t program;
  source_info_t source_info;
  int32_t dec_index = -1;
  int32_t enc_index = -1;

  program.video_stream = 1;
  source_info.programs.push_back(program);
  source_info.video_streams.push_back(video_stream_info);

  resource_info.sourceInfo = &source_info;
  resource_info.result = true;

  std::lock_guard<std::mutex> lock(mutex_);
  if (requestor_) {
    requestor_->NotifyForeground();
    requestor_->RegisterUMSPolicyActionCallback([this]() {
      requestor_->NotifyBackground();
    });

  if (!requestor_->AcquireResources(resourceMMap,
           source_info, resources)) {
    MCIL_INFO_PRINT("Resource acquisition failed");
    return false;
  }

  for (auto it : resourceMMap) {
     MCIL_DEBUG_PRINT("Resource::[%s]=>index:%d", it.first.c_str(), it.second);
     if (it.first == "VDEC") {
       dec_index = it.second;
     }
     else if (it.first == "VENC") {
       enc_index = it.second;
     }
  }

  if (dec_index != -1 &&
      vdec_index_list_.find(dec_index) == vdec_index_list_.end()) {
    MCIL_DEBUG_PRINT(" Decoder resource acquired:%d", dec_index);
    vdec_index_list_.insert(dec_index);
    *vdec_index = dec_index;
  }

  if (enc_index != -1 &&
      venc_index_list_.find(enc_index) == venc_index_list_.end()) {
    MCIL_DEBUG_PRINT(" Encoder resource acquired:%d", enc_index);
    venc_index_list_.insert(enc_index);
    *vdec_index = enc_index;
  }

  } else {
    MCIL_INFO_PRINT(" failed creating requestor_");
    return false;
  }

  if (*vdec_index < 0)
    return false;

  return true;
}

bool VideoResource::Release(DeviceType device_type,
                            std::string& resources,
                            int32_t vdec_index) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (device_type == V4L2_DECODER && !vdec_index_list_.empty())
    vdec_index_list_.erase(vdec_index);

  if (device_type == V4L2_ENCODER && !venc_index_list_.empty())
    venc_index_list_.erase(vdec_index);

  if (requestor_)
    return requestor_->ReleaseResource(resources);

  return false;
}

}  // namespace decoder

}  // namespace mcil
