// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file defines the V4L2Device interface which is used by the
// V4L2DecodeAccelerator class to delegate/pass the device specific
// handling of any of the functionalities.

#ifndef SRC_IMPL_V4L2_V4L2_DEVICE_H_
#define SRC_IMPL_V4L2_V4L2_DEVICE_H_

#include "v4l2/v4l2_utils.h"

namespace mcil {

class V4L2Buffer;
class V4L2BuffersList;
class V4L2Queue;
class V4L2WritableBufferRef;

class V4L2Device : public RefCounted<V4L2Device> {
 public:
  static scoped_refptr<V4L2Device> Create(DeviceType device_type);

  static uint32_t VideoCodecProfileToV4L2PixFmt(VideoCodecProfile profile);
  static Size AllocatedSizeFromV4L2Format(const struct v4l2_format& format);
  static int32_t VideoCodecProfileToV4L2H264Profile(VideoCodecProfile profile);
  static int32_t H264LevelIdcToV4L2H264Level(uint8_t level_idc);
  static size_t GetNumPlanesOfV4L2PixFmt(uint32_t pix_fmt);
  static scoped_refptr<VideoFrame> VideoFrameFromV4L2Format(
      const struct v4l2_format& format);

  virtual bool Open(DeviceType type, uint32_t v4l2_pixfmt) = 0;
  virtual int Ioctl(int request, void* arg) = 0;
  virtual bool Poll(bool poll_device, bool* event_pending) = 0;
  virtual bool SetDevicePollInterrupt() = 0;
  virtual bool ClearDevicePollInterrupt() = 0;
  virtual void* Mmap(void* addr, unsigned int len, int prot, int flags,
                     unsigned int offset) = 0;
  virtual void Munmap(void* addr, unsigned int len) = 0;
  virtual std::vector<int32_t> GetDmabufsForV4L2Buffer(
      int index, size_t num_planes, enum v4l2_buf_type buffer_type) = 0;
  virtual bool CanCreateEGLImageFrom(const Fourcc fourcc) const = 0;
  virtual unsigned int GetTextureTarget() const = 0;
  virtual SupportedProfiles GetSupportedDecodeProfiles() = 0;
  virtual SupportedProfiles GetSupportedEncodeProfiles() = 0;
  virtual void EnumerateDevicesForType(DeviceType type) = 0;
  virtual std::vector<uint32_t> PreferredInputFormat(DeviceType type) const = 0;

  virtual void GetSupportedResolution(uint32_t pixelformat,
                                      Size* min_resolution,
                                      Size* max_resolution);
  virtual std::vector<uint32_t> EnumerateSupportedPixelformats(
      v4l2_buf_type buf_type);
  virtual std::vector<VideoCodecProfile> V4L2PixFmtToVideoCodecProfiles(
      uint32_t pix_fmt);
  virtual Optional<struct v4l2_event> DequeueEvent();
  virtual Optional<struct v4l2_ext_control> GetCtrl(uint32_t ctrl_id);
  virtual bool SetCtrl(uint32_t ctrl_class, uint32_t ctrl_id, int32_t ctrl_val);
  virtual bool IsCtrlExposed(uint32_t ctrl_id);
  virtual bool SetGOPLength(uint32_t gop_length);

  bool IsDecoder();
  scoped_refptr<V4L2Queue> GetQueue(enum v4l2_buf_type buffer_type);

 protected:
  friend class RefCounted<V4L2Device>;
  using Devices = std::vector<std::pair<std::string, std::vector<uint32_t>>>;

  V4L2Device(DeviceType device_type);
  virtual ~V4L2Device() noexcept(false);

  SupportedProfiles EnumerateSupportedDecodeProfiles();
  SupportedProfiles EnumerateSupportedEncodeProfiles();

 private:
  virtual bool Initialize() = 0;

  void OnQueueDestroyed(enum v4l2_buf_type buffer_type);

  DeviceType device_type_ = V4L2_DECODER;

  // Associates a v4l2_buf_type to its queue.
  std::map<enum v4l2_buf_type, V4L2Queue*> queues_;
}; /* V4L2Device */

}  //  namespace mcil

#endif  // SRC_IMPL_V4L2_V4L2_DEVICE_H_
