// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file contains an implementation of VideoDecodeAccelerator
// that utilizes hardware video decoders, which expose Video4Linux 2 API
// (http://linuxtv.org/downloads/v4l-dvb-apis/).

#ifndef SRC_IMPL_V4L2_V4L2_VIDEO_DECODER_H_
#define SRC_IMPL_V4L2_V4L2_VIDEO_DECODER_H_

#include "base/thread.h"
#include "base/video_decoder.h"

#include "v4l2/v4l2_buffers.h"
#include "v4l2/v4l2_utils.h"

namespace mcil {

class Fourcc;
class V4L2Device;

class V4L2VideoDecoder : public VideoDecoder {
 public:
  static scoped_refptr<VideoDecoder> Create();
  static SupportedProfiles GetSupportedProfiles();

  V4L2VideoDecoder();
  ~V4L2VideoDecoder() override;

  bool Initialize(const DecoderConfig* config,
                  VideoDecoderClient* client,
                  DecoderClientConfig* client_config,
                  int32_t vdec_port_index) override;
  void Destroy() override;
  bool ResetInputBuffer() override;
  bool ResetDecodingBuffers(bool* reset_pending) override;
  bool CanNotifyResetDone() override;

  bool DecodeBuffer(const void* buffer,
                    size_t buffer_size,
                    const int32_t buffer_id,
                    int64_t buffer_pts) override;
  bool FlushInputBuffers() override;
  bool DidFlushBuffersDone() override;

  void EnqueueBuffers() override;
  void DequeueBuffers() override;
  void ReusePictureBuffer(int32_t pic_buffer_id) override;

  void RunDecodeBufferTask(bool event_pending, bool has_output) override;

  void SetDecoderState(CodecState state) override;
  bool GetCurrentInputBufferId(int32_t* buffer_id) override;
  size_t GetFreeBuffersCount(QueueType queue_type) override;
  bool AllocateOutputBuffers(
      uint32_t count, std::vector<WritableBufferRef*>& buffers) override;
  bool CanCreateEGLImageFrom(VideoPixelFormat pixel_format) override;
  void OnEGLImagesCreationCompleted() override;
  void RunDecoderPostTask(PostTaskType task, bool value) override {}
  #if defined (ENABLE_REACQUIRE)
  void SetResolutionChangeCb(ResolutionChangeCb cb) override;
  #endif

  void DevicePollTask(bool poll_device);

 protected:
  // These are rather subjectively tuned.
  enum {
    kInputBufferCount = 8,
    kInputBufferMaxSizeFor1080p = 1024 * 1024,
    kInputBufferMaxSizeFor4k = 4 * kInputBufferMaxSizeFor1080p,
    kDpbOutputBufferExtraCount = 5,
    kDpbOutputBufferExtraCountForImageProcessor = 1,
  };

  virtual bool IsDecoderCmdSupported();
  virtual bool SendDecoderCmdStop();
  virtual bool SendDecoderCmdStart();

  virtual bool CheckConfig(const DecoderConfig* config);
  virtual bool SetupFormats();

  virtual bool SubscribeEvents();
  virtual bool UnsubscribeEvents();
  virtual int32_t DequeueResolutionChangeEvent();

  virtual bool AllocateInputBuffers();
  virtual bool CreateOutputBuffers();
  virtual bool DestroyOutputBuffers();
  virtual void DestroyInputBuffers();

  virtual bool GetFormatInfo(struct v4l2_format* format,
                             Size* visible_size, bool* again);
  virtual Size GetVisibleSize(const Size& coded_size);
  virtual bool CreateBuffersForFormat(const struct v4l2_format& format,
                                      const Size& visible_size);

  virtual void NotifyErrorState(DecoderError error);

  virtual bool EnqueueInputBuffer(V4L2WritableBufferRef buffer);
  virtual bool DequeueInputBuffer();

  virtual bool EnqueueOutputBuffer(V4L2WritableBufferRef buffer);
  virtual bool DequeueOutputBuffer();

  virtual bool StartDevicePoll();
  virtual bool StopDevicePoll();

  virtual bool StopInputStream();
  virtual bool StopOutputStream();

  virtual void StartResolutionChange();
  virtual void FinishResolutionChange();

  scoped_refptr<V4L2Device> v4l2_device_;

  Optional<V4L2WritableBufferRef> current_input_buffer_;

  scoped_refptr<V4L2Queue> input_queue_;
  scoped_refptr<V4L2Queue> output_queue_;

  std::queue<V4L2WritableBufferRef> input_ready_queue_;

  OutputMode output_mode_ = OUTPUT_ALLOCATE;

  bool decoder_cmd_supported_ = false;
  bool flush_awaiting_last_output_buffer_ = false;

  uint32_t input_format_fourcc_ = 0;

  Optional<Fourcc> output_format_fourcc_;

  Optional<Fourcc> egl_image_format_fourcc_;

  Size coded_size_;
  Size visible_size_;

  int32_t output_dpb_size_ = 0;

  DecoderConfig decoder_config_ = {0};

  VideoDecoderClient* client_ = nullptr;

  ChronoTime start_time_;
  uint32_t frames_per_sec_ = 0;
  uint32_t current_secs_ = 0;

  #if defined (ENABLE_REACQUIRE)
  ResolutionChangeCb resolution_change_cb_ = nullptr;
  #endif

  std::atomic<uint32_t> enqueued_output_buffers_{0};

  Thread device_poll_thread_;

  std::atomic<CodecState> decoder_state_{kUninitialized};
};

#define NOTIFY_ERROR(x)                      \
  do {                                       \
    NotifyErrorState(x);                     \
  } while (0)

#define IOCTL_OR_ERROR_RETURN_VALUE(type, arg, value, error_str) \
  do { \
    if (v4l2_device_->Ioctl(type, arg) != 0) { \
      MCIL_ERROR_PRINT(": ioctl() failed: %s", error_str); \
      NOTIFY_ERROR(PLATFORM_FAILURE); \
      return value; \
    } \
  } while (0)

#define IOCTL_OR_ERROR_RETURN_FALSE(type, arg) \
  IOCTL_OR_ERROR_RETURN_VALUE(type, arg, false, #type)

}  // namespace mcil

#endif  // SRC_IMPL_V4L2_V4L2_VIDEO_DECODER_H_
