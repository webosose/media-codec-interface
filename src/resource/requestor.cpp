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

#include "requestor.h"

#include <boost/regex.hpp>

#include <string>
#include <set>
#include <utility>
#include <cmath>

#include "base/log.h"

#if !defined(ENABLE_WRAPPER)
using mrc::ResourceCalculator;
#endif

namespace mcil {

ResourceRequestor::ResourceRequestor(const std::string& connectionId)
  : rc_(std::shared_ptr<MRC>(MRC::create())),
    cb_(nullptr),
    videoResData_{VIDEO_CODEC_NONE, VIDEO_CODEC_NONE, VIDEO_CODEC_NONE,
                  0, 0, 0, 0, 0, 0, 0},
    allowPolicy_(true) {
  try {
    if (connectionId.empty()) {
      umsRMC_ = std::make_shared<uMediaServer::ResourceManagerClient> ();
      umsRMC_->registerPipeline("media"); // only rmc case
      connectionId_ = ((umsRMC_->getConnectionID() != nullptr)
                          ? std::string(umsRMC_->getConnectionID())
                          : std::string());  // after registerPipeline
      if (connectionId_.empty()) {
        MCIL_ERROR_PRINT("Failed to get connection ID");
       return;
      }
    } else {
      umsRMC_ =
          std::make_shared<uMediaServer::ResourceManagerClient>(connectionId);
      connectionId_ = connectionId;
    }
  } catch (const std::exception &e) {
    MCIL_ERROR_PRINT("Failed to create ResourceRequestor [%s]", e.what());
   return;
  }

  if (connectionId_.empty()) {
    MCIL_ERROR_PRINT("Connection id is empty");
    assert(0);
  }

  umsRMC_->registerPolicyActionHandler(
      std::bind(&ResourceRequestor::PolicyActionHandler, this,
                std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3, std::placeholders::_4,
                std::placeholders::_5));
  MCIL_DEBUG_PRINT("ResourceRequestor creation done");
}

ResourceRequestor::~ResourceRequestor() { }

bool ResourceRequestor::AcquireResources(PortResource_t& resourceMMap,
                                         const source_info_t &sourceInfo,
                                         std::string& resources) {
  std::string payload = GetSourceString(sourceInfo);
  if (payload.empty()) {
    MCIL_ERROR_PRINT("Fail to getSourceString");
    return false;
  }

  MCIL_DEBUG_PRINT("payload string = %s", payload.c_str());

  std::string response;
  if (!umsRMC_->acquire(payload, response)) {
    MCIL_ERROR_PRINT("fail to acquire!!! response : %s", response.c_str());
    return false;
  }

  try {
    ParsePortInformation(response, resourceMMap);
    ParseResources(response, resources);
  } catch (const std::runtime_error & err) {
    MCIL_ERROR_PRINT(" err=%s, response:%s", err.what(), response.c_str());
    return false;
  }

  return true;
}

#if defined (ENABLE_REACQUIRE)
bool ResourceRequestor::ReacquireResources(PortResource_t& resourceMMap,
                                           const source_info_t &sourceInfo,
                                           std::string& resources) {
  std::string request = GetSourceString(sourceInfo);
  if (request.empty()) {
    MCIL_ERROR_PRINT("Fail to getSourceString");
    return false;
  }
  jvalue_ref obj = jobject_create_var(
      jkeyval(J_CSTR_TO_JVAL("new"), jstring_create(request.c_str())),
      jkeyval(J_CSTR_TO_JVAL("old"), jstring_create(resources.c_str())),
      J_END_OBJ_DECL);

  std::string payload;
  const char* pload = jvalue_stringify(obj);
  if (!pload) {
    MCIL_ERROR_PRINT("fail to acquire!!!");
    j_release(&obj);
    return false;
  }

  payload = pload;

  j_release(&obj);

  MCIL_DEBUG_PRINT("payload string = %s", payload.c_str());

  std::string response;
  // TODO: Enable for platform extensions selectively when available
#if !defined(PLATFORM_EXTENSION)
  if (!umsRMC_->reacquire(payload, response)) {
    MCIL_ERROR_PRINT("fail to acquire!!! response : %s", response.c_str());
    return false;
  }
#endif

  try {
    ParsePortInformation(response, resourceMMap);
    ParseResources(response, resources);
  } catch (const std::runtime_error & err) {
    MCIL_ERROR_PRINT(" err=%s, response:%s", err.what(), response.c_str());
    return false;
  }

  return true;
}
#endif

mrc::ResourceListOptions ResourceRequestor::CalcVdecResources() {
  mrc::ResourceListOptions vdec_resource;
  MCIL_DEBUG_PRINT("Codec type:%d",videoResData_.vdecode);
  if (videoResData_.vdecode != VIDEO_CODEC_NONE) {
    vdec_resource = rc_->calcVdecResourceOptions(
        (MRC::VideoCodecs)TranslateVideoCodec(videoResData_.vdecode),
        videoResData_.width,
        videoResData_.height,
        videoResData_.frameRate,
        (MRC::ScanType)TranslateScanType(videoResData_.escanType),
        (MRC::_3DType)Translate3DType(videoResData_.e3DType));
  }

  return vdec_resource;
}

mrc::ResourceListOptions ResourceRequestor::CalcVencResources() {
  mrc::ResourceListOptions venc_resource;

  MCIL_DEBUG_PRINT("Codec type:%d",videoResData_.vencode);
  if (videoResData_.vencode != VIDEO_CODEC_NONE) {
    venc_resource = rc_->calcVencResourceOptions(
        (MRC::VideoCodecs)TranslateVideoCodec(videoResData_.vencode),
        videoResData_.width,
        videoResData_.height,
        videoResData_.frameRate);
  }

  return venc_resource;
}

bool ResourceRequestor::ReleaseResource(std::string& resources) {
  if (resources.empty()) {
    MCIL_DEBUG_PRINT("[%s], resource already empty", __func__);
    return true;
  }

  if (!umsRMC_->release(resources)) {
    MCIL_ERROR_PRINT("release error : %s", resources.c_str());
    return false;
  }

  resources = "";
  return true;
}

bool ResourceRequestor::NotifyForeground() const {
  return umsRMC_->notifyForeground();
}

bool ResourceRequestor::NotifyBackground() const {
  return umsRMC_->notifyBackground();
}

void ResourceRequestor::AllowPolicyAction(const bool allow) {
  allowPolicy_ = allow;
}

bool ResourceRequestor::PolicyActionHandler(const char *action,
                                            const char *resources,
                                            const char *requestorType,
                                            const char *requestorName,
                                            const char *connectionId) {
  MCIL_DEBUG_PRINT("action:%s, resources:%s, type:%s, name:%s, id:%s",
      action, resources, requestorType, requestorName, connectionId);

  if (allowPolicy_) {
    if (nullptr != cb_) {
      cb_();
    }
    if (!umsRMC_->release(resources)) {
      MCIL_ERROR_PRINT("release error : %s", resources);
      return false;
    }
  }

  return allowPolicy_;
}

std::string ResourceRequestor::GetSourceString(
    const source_info_t &sourceInfo) {
  std::string payload;
  mrc::ResourceListOptions finalOptions;
  if (!SetSourceInfo(sourceInfo)) {
    MCIL_ERROR_PRINT("Failed to set source info!");
    return payload;
  }

  mrc::ResourceListOptions vdec_resource = CalcVdecResources();
  if (!vdec_resource.empty()) {
    mrc::concatResourceListOptions(&finalOptions, &vdec_resource);
    MCIL_DEBUG_PRINT("vdec_resource size:%lu, %s, %d",
        vdec_resource.size(), vdec_resource[0].front().type.c_str(),
        vdec_resource[0].front().quantity);
  }

  mrc::ResourceListOptions venc_resource = CalcVencResources();
  if (!venc_resource.empty()) {
    mrc::concatResourceListOptions(&finalOptions, &venc_resource);
    MCIL_DEBUG_PRINT("venc_resource size:%lu, %s, %d",
        venc_resource.size(), venc_resource[0].front().type.c_str(),
        venc_resource[0].front().quantity);
  }

  jvalue_ref arr = jarray_create(NULL);
  for (auto& option : finalOptions) {
    for (auto const& it : option) {
      jvalue_ref arr_obj = jobject_create_var(
          jkeyval(J_CSTR_TO_JVAL("resource"), jstring_create(it.type.c_str())),
          jkeyval(J_CSTR_TO_JVAL("qty"), jnumber_create_i32(it.quantity)),
          J_END_OBJ_DECL);
      jarray_append(arr, arr_obj);
      MCIL_DEBUG_PRINT("calculator return : %s, %d", it.type.c_str(),
                       it.quantity);
    }
  }

  payload = (jvalue_stringify(arr) != NULL) ? jvalue_stringify(arr) : "";
  if (payload.empty()) {
    MCIL_ERROR_PRINT("Failed to set source info!");
    j_release(&arr);
    return payload;
  }

  j_release(&arr);

  return payload;
}

bool ResourceRequestor::ParsePortInformation(const std::string& payload,
                                             PortResource_t& resourceMMap) {
  JSchemaInfo schema_info;
  jvalue_ref resources_obj = NULL;
  jschema_info_init(&schema_info, jschema_all(), NULL, NULL);

  jvalue_ref payload_json =
      jdom_parse(j_cstr_to_buffer(payload.c_str()), DOMOPT_NOOPT, &schema_info);
  jschema_release(&schema_info.m_schema);

  if (payload_json == jinvalid()) {
    throw std::runtime_error(" : payload parsing failure");
  }

  if (!jobject_get_exists(payload_json, J_CSTR_TO_BUF("resources"),
                          &resources_obj)) {
    j_release(&payload_json);
    throw std::runtime_error("payload must have \"resources key\"");
  }

  for (ssize_t i = 0; i < jarray_size(resources_obj); ++i) {
    jvalue_ref item = jarray_get(resources_obj, i);
    jvalue_ref resource;
    jobject_get_exists(item, J_CSTR_TO_BUF("resource"), &resource);

    raw_buffer resource_string = jstring_get(resource);
    std::string res(resource_string.m_str, resource_string.m_len);
    jstring_free_buffer(resource_string);

    jvalue_ref index;
    jobject_get_exists(item, J_CSTR_TO_BUF("index"), &index);
    int32_t ind(0);
    jnumber_get_i32(index, &ind);

    resourceMMap.insert<>(std::make_pair(res, ind));
  }

  j_release(&payload_json);

  for (auto& it : resourceMMap) {
    MCIL_DEBUG_PRINT("port Resource- %s, : [%d] ", it.first.c_str(), it.second);
  }

  return true;
}

bool ResourceRequestor::ParseResources(const std::string& payload,
                                       std::string& resources) {
  resources = std::string();

  JSchemaInfo schema_info;
  jvalue_ref resources_obj = NULL;
  jschema_info_init(&schema_info, jschema_all(), NULL, NULL);

  jvalue_ref payload_json =
      jdom_parse(j_cstr_to_buffer(payload.c_str()), DOMOPT_NOOPT, &schema_info);
  jschema_release(&schema_info.m_schema);

  if (payload_json == jinvalid()) {
    throw std::runtime_error(" : payload parsing failure");
  }

  if (!jobject_get_exists(payload_json, J_CSTR_TO_BUF("resources"),
                          &resources_obj)) {
    j_release(&payload_json);
    throw std::runtime_error("payload must have \"resources key\"");
  }

  jvalue_ref arr = jarray_create(NULL);
  for (ssize_t i = 0; i < jarray_size(resources_obj); ++i) {
    jvalue_ref item = jarray_get(resources_obj, i);
    jvalue_ref resource;
    jobject_get_exists(item, J_CSTR_TO_BUF("resource"), &resource);

    raw_buffer resource_string = jstring_get(resource);
    std::string res(resource_string.m_str, resource_string.m_len);
    jstring_free_buffer(resource_string);

    jvalue_ref index;
    jobject_get_exists(item, J_CSTR_TO_BUF("index"), &index);
    int32_t ind(0);
    jnumber_get_i32(index, &ind);

    jvalue_ref arr_obj = jobject_create_var(
        jkeyval(J_CSTR_TO_JVAL("index"), jnumber_create_i32(ind)),
        jkeyval(J_CSTR_TO_JVAL("resource"), jstring_create(res.c_str())),
        J_END_OBJ_DECL);
    jarray_append(arr, arr_obj);
  }

  resources = (jvalue_stringify(arr) != NULL) ? jvalue_stringify(arr) : "";

  j_release(&arr);
  j_release(&payload_json);

  return true;
}

int32_t ResourceRequestor::TranslateVideoCodec(const VideoCodec vcodec) const {
  MRC::VideoCodec ev = MRC::kVideoEtc;
  switch (vcodec) {
    case VIDEO_CODEC_NONE:
      ev = MRC::kVideoEtc;    break;
    case VIDEO_CODEC_H264:
      ev = MRC::kVideoH264;   break;
    case VIDEO_CODEC_HEVC:
      ev = MRC::kVideoH265;   break;
    case VIDEO_CODEC_MPEG2:
      ev = MRC::kVideoMPEG;   break;
    case VIDEO_CODEC_MPEG4:
      ev = MRC::kVideoMPEG4;  break;
    case VIDEO_CODEC_VP8:
      ev = MRC::kVideoVP8;    break;
    case VIDEO_CODEC_VP9:
      ev = MRC::kVideoVP9;    break;
    default:
      ev = MRC::kVideoH264;   break;
  }

  return static_cast<int32_t>(ev);
}

int32_t ResourceRequestor::TranslateScanType(const int32_t escanType) const {
  MRC::ScanType scan = MRC::kScanProgressive;

  switch (escanType) {
    default:
      break;
  }

  return static_cast<int32_t>(scan);
}

int32_t ResourceRequestor::Translate3DType(const int32_t e3DType) const {
  MRC::_3DType my3d = MRC::k3DNone;

  switch (e3DType) {
    default:
      my3d = MRC::k3DNone;
      break;
  }

  return static_cast<int32_t>(my3d);
}

bool ResourceRequestor::SetSourceInfo(const source_info_t &sourceInfo) {
  if (sourceInfo.video_streams.empty()){
    MCIL_ERROR_PRINT("Video/Audio streams are empty!");
    return false;
  }

  video_info_t video_stream_info = sourceInfo.video_streams.front();
  if ((video_stream_info.encode < VIDEO_CODEC_NONE) || (video_stream_info.encode > VIDEO_CODEC_MAX) ||
      (video_stream_info.decode < VIDEO_CODEC_NONE) || (video_stream_info.decode > VIDEO_CODEC_MAX)) {
    MCIL_ERROR_PRINT("Invalid Video/Audio Codec!");
    return false;
  }

  videoResData_.width = video_stream_info.width;
  videoResData_.height = video_stream_info.height;
  videoResData_.vencode = (VideoCodec)video_stream_info.encode;
  videoResData_.vdecode = (VideoCodec)video_stream_info.decode;
  videoResData_.frameRate = static_cast<int32_t>(
      std::round(static_cast<float>(video_stream_info.frame_rate.num) /
                 static_cast<float>(video_stream_info.frame_rate.den)));
  videoResData_.escanType = 0;

  return true;
}

}  // namespace mcil
