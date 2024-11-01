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

#ifndef SRC_IMPL_GST_GST_VIDEO_ENCODER_H_
#define SRC_IMPL_GST_GST_VIDEO_ENCODER_H_

#include "base/video_encoder.h"
#include "gst_wrapper/buffer_encoder_wrapper.h"

#if !defined(ENABLE_WRAPPER)
namespace mrf { class BufferEncoder; }
#endif

namespace mcil {

class GstVideoEncoder : public VideoEncoder {
 public:
  static scoped_refptr<VideoEncoder> Create();
  static SupportedProfiles GetSupportedProfiles();

  GstVideoEncoder();
  virtual ~GstVideoEncoder() override;

  virtual bool Initialize(
      const EncoderConfig* config, VideoEncoderClient* client,
      EncoderClientConfig* client_config, int32_t venc_port_index) override;
  virtual void Destroy() override;
  virtual bool EncodeBuffer(
      const uint8_t* yBuf, size_t ySize, const uint8_t* uBuf, size_t uSize,
      const uint8_t* vBuf, size_t vSize, uint64_t bufferTimestamp,
      const bool requestKeyFrame) override;
  virtual bool UpdateEncodingParams(uint32_t bitrate, uint32_t framerate)
      override;

  virtual bool IsFlushSupported() override {
    return false;
  }
  virtual bool EncodeFrame(scoped_refptr<VideoFrame> frame, bool force_keyframe)
      override {
    return true;
  }
  virtual bool FlushFrames() override {
    return true;
  }
  virtual bool StartDevicePoll() override {
    return true;
  }
  virtual void RunEncodeBufferTask() override {}
  virtual void SendStartCommand(bool start) override {}
  virtual void SetEncoderState(CodecState state) override {}
  virtual size_t GetFreeBuffersCount(QueueType queue_type) override {
    return 0;
  }
  virtual void EnqueueBuffers() override {}
  virtual scoped_refptr<VideoFrame> GetDeviceInputFrame() override {
    return nullptr;
  }
  virtual bool NegotiateInputFormat(VideoPixelFormat format,
                                    const Size& frame_size) override {
    return true;
  }

  void OnEncodedBuffer(const uint8_t* data, size_t size,
                       uint64_t timestamp, bool is_keyframe);

 private:
  mrf::BufferEncoder gst_pipeline_;
  VideoEncoderClient* client_ = nullptr;
};

}  // namespace mcil

#endif  // SRC_IMPL_GST_GST_VIDEO_ENCODER_H_
