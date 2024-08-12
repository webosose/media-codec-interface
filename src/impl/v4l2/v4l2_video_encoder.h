// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_IMPL_V4L2_V4L2_VIDEO_ENCODER_H_
#define SRC_IMPL_V4L2_V4L2_VIDEO_ENCODER_H_

#include "base/thread.h"
#include "base/video_encoder.h"

#include "v4l2/v4l2_buffers.h"
#include "v4l2/v4l2_utils.h"

namespace mcil {

class Fourcc;
class V4L2Device;

class V4L2VideoEncoder : public VideoEncoder {
 public:
  static scoped_refptr<VideoEncoder> Create();
  static SupportedProfiles GetSupportedProfiles();

  V4L2VideoEncoder();
  ~V4L2VideoEncoder() override;

  bool Initialize(const EncoderConfig* config,
                  VideoEncoderClient* client,
                  EncoderClientConfig* client_config,
                  int32_t venc_port_index) override;
  void Destroy() override;
  bool IsFlushSupported() override;
  bool EncodeFrame(scoped_refptr<VideoFrame> frame,
                   bool force_keyframe) override;
  bool FlushFrames() override;
  bool UpdateEncodingParams(uint32_t bitrate, uint32_t framerate) override;
  bool StartDevicePoll() override;
  void RunEncodeBufferTask() override;
  void SendStartCommand(bool start) override;
  void SetEncoderState(CodecState state) override;
  size_t GetFreeBuffersCount(QueueType queue_type) override;
  void EnqueueBuffers() override;
  scoped_refptr<VideoFrame> GetDeviceInputFrame() override;
  bool NegotiateInputFormat(VideoPixelFormat format,
                            const Size& frame_size) override;

 protected:
  // These are rather subjectively tuned.
  enum {
    kInputBufferCount = 2,
    kOutputBufferCount = 2,
  };

  struct InputFrameInfo {
    InputFrameInfo() = default;
    InputFrameInfo(scoped_refptr<VideoFrame> frame, bool force_keyframe);
    InputFrameInfo(const InputFrameInfo&) = default;
    ~InputFrameInfo() = default;
    scoped_refptr<VideoFrame> frame = nullptr;
    bool force_keyframe = false;
  };

  virtual void DequeueBuffers();
  virtual uint32_t GetCapsRequired();
  virtual void InitInputMemoryType();
  virtual void InitOutputMemoryType();
  virtual bool DoStreamOnInEnqueueBuffers() { return true; };
  virtual bool ShouldSetEncodingParams() { return true; };
  virtual bool ShouldApplyCrop() { return true; };
  virtual bool ShouldInjectSpsPps() { return false; };

  virtual bool SetFormats(VideoPixelFormat input_format,
                          VideoCodecProfile output_profile);

  virtual bool SetOutputFormat(VideoCodecProfile output_profile);
  virtual Optional<struct v4l2_format> SetInputFormat(
      VideoPixelFormat format, const Size& frame_size);
  virtual bool ApplyCrop();

  virtual bool InitControls(const EncoderConfig* config);
  virtual bool InitControlsH264(const EncoderConfig* config);
  virtual void InitControlsVP8(const EncoderConfig* config);

  virtual void NotifyErrorState(EncoderError error);

  virtual bool CreateInputBuffers();
  virtual bool CreateOutputBuffers();

  virtual void DestroyInputBuffers();
  virtual void DestroyOutputBuffers();

  virtual bool EnqueueInputBuffer(V4L2WritableBufferRef buffer);
  virtual bool DequeueInputBuffer();

  virtual bool EnqueueOutputBuffer(V4L2WritableBufferRef buffer);
  virtual bool DequeueOutputBuffer();

  virtual bool StopDevicePoll();
  virtual void DevicePollTask(bool poll_device);

  EncoderConfig encoder_config_ = {0};

  scoped_refptr<V4L2Device> v4l2_device_;

  scoped_refptr<V4L2Queue> input_queue_;
  scoped_refptr<V4L2Queue> output_queue_;

  bool is_flush_supported_ = false;
  bool input_buffer_created_ = false;

  Size input_frame_size_;
  Rect input_visible_rect_;

  size_t output_buffer_byte_size_ = 0;
  uint32_t output_format_fourcc_ = 0;

  size_t current_bitrate_ = 0;
  size_t current_framerate_ = 0;

  scoped_refptr<VideoFrame> device_input_frame_;
  std::queue<InputFrameInfo> encoder_input_queue_;

  v4l2_memory input_memory_type_;
  v4l2_memory output_memory_type_;
  bool inject_sps_and_pps_ = false;

  Thread device_poll_thread_;

  VideoEncoderClient* client_ = nullptr;

  ChronoTime start_time_;
  uint32_t frames_per_sec_ = 0;
  uint32_t current_secs_ = 0;

  std::atomic<CodecState> encoder_state_{kUninitialized};
};

#define NOTIFY_ERROR(x)                      \
  do {                                       \
    NotifyErrorState(x);                     \
  } while (0)

#define IOCTL_OR_ERROR_RETURN_VALUE(type, arg, value, error_str) \
  do { \
    if (v4l2_device_->Ioctl(type, arg) != 0) { \
      MCIL_ERROR_PRINT(": ioctl() failed: %s", error_str); \
      NOTIFY_ERROR(kPlatformFailureError); \
      return value; \
    } \
  } while (0)

#define IOCTL_OR_ERROR_RETURN(type, arg) \
  IOCTL_OR_ERROR_RETURN_VALUE(type, arg, ((void)0), #type)

#define IOCTL_OR_ERROR_RETURN_FALSE(type, arg) \
  IOCTL_OR_ERROR_RETURN_VALUE(type, arg, false, #type)

}  // namespace mcil

#endif  // SRC_IMPL_V4L2_V4L2_VIDEO_ENCODER_H_
