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

#include "gst_video_encoder.h"

#include <cstring>
#include <cstdlib>
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
#include "base/video_encoder_client.h"

#define GST_V4L2_ENCODER

namespace mcil {

// static
scoped_refptr<VideoEncoder> GstVideoEncoder::Create() {
  return new GstVideoEncoder();
}

// static
SupportedProfiles GstVideoEncoder::GetSupportedProfiles() {
  SupportedProfiles supported_profiles;

  SupportedProfile supported_profile;
  supported_profile.profile = H264PROFILE_BASELINE;
  supported_profile.max_resolution = Size(1920, 1080);
  supported_profile.min_resolution = Size(1, 1);
  supported_profiles.push_back(supported_profile);

  return supported_profiles;
}


GstVideoEncoder::GstVideoEncoder()
 : VideoEncoder() {
}

GstVideoEncoder::~GstVideoEncoder() {
  if (pipeline_)
    gst_element_set_state(pipeline_, GST_STATE_NULL);
}

bool GstVideoEncoder::Initialize(const EncoderConfig* config,
                                  VideoEncoderClient* client,
                                  EncoderClientConfig* client_config,
                                  int32_t venc_port_index) {
  client_ = client;
  if (!client_) {
    MCIL_ERROR_PRINT(" Delegate not provided");
    return false;
  }

  if (!CreatePipeline(config)) {
    MCIL_ERROR_PRINT("CreatePipeline Failed");
    return false;
  }

  bitrate_ = 0;

  if (client_config) {
    client_config->input_frame_size = Size(config->width,
                                           config->height);
    client_config->should_control_buffer_feed = true;
    client_config->should_inject_sps_and_pps = false;
  }

  return true;
}

void GstVideoEncoder::Destroy() {
  if (pipeline_) {
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    pipeline_ = nullptr;
  }
}

bool GstVideoEncoder::EncodeBuffer(const uint8_t* yBuf, size_t ySize,
                                   const uint8_t* uBuf, size_t uSize,
                                   const uint8_t* vBuf, size_t vSize,
                                   uint64_t bufferTimestamp,
                                   const bool requestKeyFrame) {
  if (!pipeline_) {
    MCIL_ERROR_PRINT("Pipeline is null");
    return false;
  }

  size_t bufferSize = ySize + uSize * 2;
  guint8 *feedBuffer = (guint8 *)g_malloc(bufferSize);
  if (feedBuffer == NULL) {
    MCIL_ERROR_PRINT("memory allocation error!!!!!");
    return false;
  }

  memcpy(feedBuffer, yBuf, ySize);
  memcpy(feedBuffer + ySize, uBuf, uSize);
  memcpy(feedBuffer + ySize + uSize, vBuf, vSize);

  GstBuffer *gstBufferWrap = gst_buffer_new_wrapped(feedBuffer, bufferSize);
  if (!gstBufferWrap) {
    MCIL_ERROR_PRINT("Buffer wrapping error");
    return false;
  }

  GstFlowReturn gstReturn = gst_app_src_push_buffer((GstAppSrc*)source_,
                                                    gstBufferWrap);
  if (gstReturn < GST_FLOW_OK) {
    MCIL_ERROR_PRINT("gst_app_src_push_buffer errCode[ %d ]", gstReturn);
    return false;
  }
  return true;
}

bool GstVideoEncoder::UpdateEncodingParams(uint32_t bitrate,
                                           uint32_t framerate) {
  MCIL_DEBUG_PRINT(": bitrate=%d, framerate=%d", bitrate, framerate);

  if (encoder_ && bitrate > 0 && bitrate_ != bitrate) {
#if defined(GST_V4L2_ENCODER)
    GstStructure* extraCtrls = gst_structure_new ("extra-controls",
                               "video_bitrate", G_TYPE_INT, bitrate, NULL);
    g_object_set(G_OBJECT(encoder_), "extra-controls", extraCtrls, NULL);
#else
    g_object_set(G_OBJECT(encoder_), "target-bitrate", bitrate, NULL);
#endif
    bitrate_ = bitrate;
  }
  return true;
}

bool GstVideoEncoder::CreateEncoder(VideoCodecProfile profile) {
  MCIL_DEBUG_PRINT(" profile: %d", profile);

  if (profile >= H264PROFILE_MIN && profile <= H264PROFILE_MAX) {
#if defined(GST_V4L2_ENCODER)
    encoder_ = gst_element_factory_make ("v4l2h264enc", "encoder");
#else
    encoder_ = gst_element_factory_make ("omxh264enc", "encoder");
#endif
  } else if (profile >= VP8PROFILE_MIN && profile <= VP8PROFILE_MAX) {
    encoder_ = gst_element_factory_make ("omxvp8enc", "encoder");
  } else {
    MCIL_ERROR_PRINT(": Unsupported Codedc");
    return false;
  }

  if (!encoder_) {
    MCIL_ERROR_PRINT("encoder_ element creation failed.");
    return false;
  }
  return true;
}

bool GstVideoEncoder::CreateSink() {
  sink_ = gst_element_factory_make ("appsink", "sink");
  if (!sink_) {
    MCIL_ERROR_PRINT("sink_ element creation failed.");
    return false;
  }
  g_object_set (G_OBJECT (sink_), "emit-signals", TRUE, "sync", FALSE, NULL);
  g_signal_connect(sink_, "new-sample", G_CALLBACK(OnEncodedBuffer), this);

  return true;
}

bool GstVideoEncoder::LinkElements(const EncoderConfig* configData) {
  MCIL_DEBUG_PRINT(": width: %d, height: %d",
                   configData->width, configData->height);

  filter_YUY2_ = gst_element_factory_make("capsfilter", "filter-YUY2");
  if (!filter_YUY2_) {
    MCIL_ERROR_PRINT("filter_YUY2_(%p) Failed", filter_YUY2_);
    return false;
  }

  caps_YUY2_ = gst_caps_new_simple("video/x-raw",
                                   "width", G_TYPE_INT, configData->width,
                                   "height", G_TYPE_INT, configData->height,
                                   "framerate", GST_TYPE_FRACTION,
                                                configData->frameRate, 1,
                                   "format", G_TYPE_STRING, "I420",
                                   NULL);
  g_object_set(G_OBJECT(filter_YUY2_), "caps", caps_YUY2_, NULL);

#if defined(GST_V4L2_ENCODER)
  MCIL_DEBUG_PRINT("Do not use NV12 caps filter for V4L2");
#else
  filter_NV12_ = gst_element_factory_make("capsfilter", "filter-NV");
  if (!filter_NV12_) {
    MCIL_ERROR_PRINT("filter_ element creation failed.");
    return false;
  }

  caps_NV12_ = gst_caps_new_simple("video/x-raw",
                                   "format", G_TYPE_STRING, "NV12",
                                   NULL);
  g_object_set(G_OBJECT(filter_NV12_), "caps", caps_NV12_, NULL);
#endif

  converter_ = gst_element_factory_make("videoconvert", "converted");
  if (!converter_) {
    MCIL_ERROR_PRINT("converter_(%p) Failed", converter_);
    return false;
  }

  parse_ = gst_element_factory_make("rawvideoparse", "parser");
  if (!parse_) {
    MCIL_ERROR_PRINT("parse_(%p) Failed", parse_);
    return false;
  }

  g_object_set(G_OBJECT(parse_), "width", configData->width, NULL);
  g_object_set(G_OBJECT(parse_), "height", configData->height, NULL);
  if (PIXEL_FORMAT_I420 == configData->pixelFormat) {
    g_object_set(G_OBJECT(parse_), "format", 2, NULL);
  }

#if defined(GST_V4L2_ENCODER)
  gst_bin_add_many(GST_BIN(pipeline_), source_, filter_YUY2_, parse_,
                   converter_, encoder_, sink_, NULL);
#else
  gst_bin_add_many(GST_BIN(pipeline_), source_, filter_YUY2_, parse_,
                   converter_, filter_NV12_, encoder_, sink_, NULL);
#endif

  if (!gst_element_link(source_, filter_YUY2_)) {
    MCIL_ERROR_PRINT ("Linkerror - source_ & filter_YUY2");
    return false;
  }

  if (!gst_element_link(filter_YUY2_, parse_)) {
    MCIL_ERROR_PRINT ("Link error - filter_YUY2 & converter_");
      return false;
  }

  if (!gst_element_link(parse_, converter_)) {
    MCIL_ERROR_PRINT ("Link error - parse_ & converter_");
    return false;
  }

#if defined(GST_V4L2_ENCODER)
  if (!gst_element_link(converter_, encoder_)) {
    MCIL_ERROR_PRINT ("Link error - converter_ & encoder_");
    return false;
  }
#else
  if (!gst_element_link(converter_, filter_NV12_)) {
    MCIL_ERROR_PRINT ("Link error - converter_ & filter_NV12_");
    return false;
  }

  if (!gst_element_link(filter_NV12_, encoder_)) {
    MCIL_ERROR_PRINT ("Link error - filter_NV12_ & encoder_");
    return false;
  }
#endif

  if (!gst_element_link(encoder_, sink_)) {
    MCIL_ERROR_PRINT ("Link error - encoder_ & sink_");
    return false;
  }

  return true;
}

gboolean GstVideoEncoder::HandleBusMessage(
    GstBus *bus_, GstMessage *message, gpointer user_data) {
  GstMessageType messageType = GST_MESSAGE_TYPE(message);
  if (messageType != GST_MESSAGE_QOS && messageType != GST_MESSAGE_TAG) {
    MCIL_DEBUG_PRINT("Element[ %s ][ %d ][ %s ]",
                     GST_MESSAGE_SRC_NAME(message),
                     messageType, gst_message_type_get_name(messageType));
  }
  return true;
}

bool GstVideoEncoder::CreatePipeline(const EncoderConfig* configData) {
  gst_init(NULL, NULL);
  gst_pb_utils_init();
  pipeline_ = gst_pipeline_new("video-encoder");
  if (!pipeline_) {
    MCIL_ERROR_PRINT("Cannot create encoder pipeline!");
    return false;
  }

  source_ = gst_element_factory_make ("appsrc", "app-source");
  if (!source_) {
    MCIL_ERROR_PRINT("source_ element creation failed.");
    return false;
  }

  g_object_set(source_, "format", GST_FORMAT_TIME, NULL);
  g_object_set(source_, "do-timestamp", true, NULL);

  if (!CreateEncoder(configData->profile)) {
    MCIL_ERROR_PRINT("Encoder creation failed !!!");
    return false;
  }

  if (!CreateSink()) {
    MCIL_ERROR_PRINT("Sink creation failed !!!");
    return false;
  }

  if (!LinkElements(configData)) {
    MCIL_ERROR_PRINT("element linking failed !!!");
    return false;
  }

  bus_ = gst_pipeline_get_bus(GST_PIPELINE (pipeline_));
  gst_bus_add_watch(bus_, GstVideoEncoder::HandleBusMessage, this);
  gst_object_unref(bus_);

  return gst_element_set_state(pipeline_, GST_STATE_PLAYING);
}

/* called when the appsink notifies us that there is a new buffer ready for
 * processing */
GstFlowReturn GstVideoEncoder::OnEncodedBuffer(
    GstElement* elt, gpointer* data) {
  GstVideoEncoder *encoder = reinterpret_cast<GstVideoEncoder*>(data);
  GstSample *sample;
  GstBuffer *app_buffer;
  GstBuffer *buffer;
  GstElement *source;
  GstFlowReturn ret;

  /* get the sample from appsink */
  sample = gst_app_sink_pull_sample (GST_APP_SINK (elt));
  if (NULL != sample) {
    GstMapInfo map_info = {};
    GstBuffer *buffer;
    buffer = gst_sample_get_buffer(sample);
    gst_buffer_map(buffer, &map_info, GST_MAP_READ);

    if ((NULL == map_info.data) && (map_info.size == 0)) {
      MCIL_DEBUG_PRINT(": Empty buffer received");
      return GST_FLOW_OK;
    }

    bool is_keyframe = false;
    if(!GST_BUFFER_FLAG_IS_SET(buffer, GST_BUFFER_FLAG_DELTA_UNIT)){
      is_keyframe = true;
    }

    if (encoder->start_time_ == ChronoTime())
      encoder->start_time_ = std::chrono::system_clock::now();

    encoder->buffers_per_sec_++;
    std::chrono::duration<double> time_past =
        std::chrono::system_clock::now() - encoder->start_time_;
    if (time_past >= std::chrono::seconds(1)) {
      encoder->current_seconds_++;
      MCIL_INFO_PRINT(": Encoder @ %d secs => %d fps",
                      encoder->current_seconds_, encoder->buffers_per_sec_);
      encoder->start_time_ = std::chrono::system_clock::now();
      encoder->buffers_per_sec_ = 0;
    }

    uint64_t timestamp = GST_BUFFER_TIMESTAMP(buffer);
    encoder->client_->BitstreamBufferReady(
        static_cast<const uint8_t*>(map_info.data),
        map_info.size, timestamp, is_keyframe);

    gst_sample_unref(sample);
    gst_buffer_unmap(buffer, &map_info);
  }
  return GST_FLOW_OK;
}

}  // namespace mcil
