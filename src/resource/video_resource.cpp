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

namespace mcil {

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
                            VideoCodec video_codec,
                            uint32_t frame_width,
                            uint32_t frame_height,
                            uint32_t frame_rate,
                            std::string& resources,
                            int32_t *resource_index) {
  MCIL_DEBUG_PRINT("[video info] width: %d, height: %d, frame_rate: %d",
                   frame_width, frame_height, frame_rate);

#if defined(PLATFORM_EXTENSION)
  // TODO: Enable for platform extensions when available
  if (device_type == V4L2_ENCODER) {
    *resource_index = 1;
    return true;
  }
#endif

  if (!requestor_) {
    MCIL_ERROR_PRINT(" failed creating requestor_");
    return false;
  }

  source_info_t source_info;
  if (!GetSourceInfo(device_type, video_codec, frame_width, frame_height,
                     frame_rate, &source_info)) {
    return false;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  requestor_->NotifyForeground();
  requestor_->RegisterUMSPolicyActionCallback([this]() {
      requestor_->NotifyBackground();
    });

  PortResource_t resourceMMap;
  if (!requestor_->AcquireResources(resourceMMap, source_info, resources)) {
    MCIL_ERROR_PRINT("Resource acquisition failed");
    return false;
  }

  if (!AddToIndexList(resourceMMap, resource_index)) {
    MCIL_ERROR_PRINT("Failed to Add To Index List");
    return false;
  }

  if (*resource_index < 0) {
    MCIL_ERROR_PRINT(" resource_index = %d", *resource_index);
    return false;
  }

  return true;
}

bool VideoResource::Reacquire(DeviceType device_type,
                              VideoCodec video_codec,
                              uint32_t frame_width,
                              uint32_t frame_height,
                              uint32_t frame_rate,
                              std::string& resources,
                              int32_t *resource_index) {
  MCIL_DEBUG_PRINT(" width: %d, height: %d, frame_rate: %d index: %d",
                   frame_width, frame_height, frame_rate, *resource_index);

  RemoveFromIndexList(device_type, *resource_index);

  if (!requestor_) {
    MCIL_ERROR_PRINT(" failed creating requestor_");
    return false;
  }

  source_info_t source_info;
  if (!GetSourceInfo(device_type, video_codec, frame_width, frame_height,
                     frame_rate, &source_info)) {
    return false;
  }

  std::lock_guard<std::mutex> lock(mutex_);
  requestor_->NotifyForeground();
  requestor_->RegisterUMSPolicyActionCallback([this]() {
      requestor_->NotifyBackground();
    });

  PortResource_t resourceMMap;
  if (!requestor_->ReacquireResources(resourceMMap, source_info, resources)) {
    MCIL_ERROR_PRINT("Resource acquisition failed");
    return false;
  }

  if (!AddToIndexList(resourceMMap, resource_index)) {
    MCIL_ERROR_PRINT("Failed to Add To Index List");
    return false;
  }

  if (*resource_index < 0) {
    MCIL_ERROR_PRINT(" resource_index = %d", *resource_index);
    return false;
  }

  return true;
}

bool VideoResource::Release(DeviceType device_type,
                            std::string& resources,
                            int32_t resource_index) {
  MCIL_DEBUG_PRINT(" type: %d, index: %d", device_type, resource_index);

  std::lock_guard<std::mutex> lock(mutex_);

  RemoveFromIndexList(device_type, resource_index);

  if (requestor_)
    return requestor_->ReleaseResource(resources);
  return false;
}

bool VideoResource::GetSourceInfo(DeviceType device_type,
                                  VideoCodec video_codec,
                                  uint32_t frame_width,
                                  uint32_t frame_height,
                                  uint32_t frame_rate,
                                  source_info_t* source_info) {
  MCIL_DEBUG_PRINT("[video info] width: %d, height: %d, frame_rate: %d",
                   frame_width, frame_height, frame_rate);

  video_info_t video_stream_info = {};
  video_stream_info.width = frame_width;
  video_stream_info.height = frame_height;
  video_stream_info.frame_rate.num = frame_rate;
  video_stream_info.frame_rate.den = 1;

  if (device_type == V4L2_DECODER) {
    video_stream_info.encode = VIDEO_CODEC_NONE;
    video_stream_info.decode = video_codec;
  } else if (device_type == V4L2_ENCODER) {
    video_stream_info.encode = video_codec;
    video_stream_info.decode = VIDEO_CODEC_NONE;
  } else {
    return false;
  }

  program_info_t program;
  program.video_stream = 1;
  source_info->programs.push_back(program);
  source_info->video_streams.push_back(video_stream_info);

  return true;
}

bool VideoResource::AddToIndexList(PortResource_t resourceMMap,
                                   int32_t *resource_index) {
  int32_t dec_index = -1;
  int32_t enc_index = -1;

  for (auto it : resourceMMap) {
     MCIL_DEBUG_PRINT("Resource::[%s]=>index:%d", it.first.c_str(), it.second);
     if (it.first == "VDEC") {
       dec_index = it.second;
       if (dec_index < 0)
         return false;
     } else if (it.first == "VENC") {
       enc_index = it.second;
       if (enc_index < 0)
         return false;
     }
  }

  if (dec_index != -1 &&
      vdec_index_list_.find(dec_index) == vdec_index_list_.end()) {
    MCIL_DEBUG_PRINT(" Decoder resource acquired:%d", dec_index);
    vdec_index_list_.insert(dec_index);
    *resource_index = dec_index;
  }

  if (enc_index != -1 &&
      venc_index_list_.find(enc_index) == venc_index_list_.end()) {
    MCIL_DEBUG_PRINT(" Encoder resource acquired:%d", enc_index);
    venc_index_list_.insert(enc_index);
    *resource_index = enc_index;
  }

  return true;
}

void VideoResource::RemoveFromIndexList(DeviceType device_type,
                                        int32_t resource_index) {
  if (device_type == V4L2_DECODER && !vdec_index_list_.empty())
    vdec_index_list_.erase(resource_index);

  if (device_type == V4L2_ENCODER && !venc_index_list_.empty())
    venc_index_list_.erase(resource_index);
}

}  // namespace mcil
