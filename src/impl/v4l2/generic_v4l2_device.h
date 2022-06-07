// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file contains the implementation of GenericV4L2Device used on
// platforms, which provide generic V4L2 video codec devices.

#ifndef SRC_IMPL_V4L2_GENERIC_V4L2_DEVICE_H_
#define SRC_IMPL_V4L2_GENERIC_V4L2_DEVICE_H_

#include "v4l2/v4l2_device.h"

namespace mcil {

class GenericV4L2Device : public V4L2Device {
 public:
  GenericV4L2Device(DeviceType device_type);

  GenericV4L2Device(const GenericV4L2Device&) = delete;
  GenericV4L2Device& operator=(const GenericV4L2Device&) = delete;

  // V4L2Device implementation.
  bool Open(DeviceType type, uint32_t v4l2_pixfmt) override;
  int Ioctl(int request, void* arg) override;
  bool Poll(bool poll_device, bool* event_pending) override;
  bool SetDevicePollInterrupt() override;
  bool ClearDevicePollInterrupt() override;
  void* Mmap(void* addr,
             unsigned int len,
             int prot,
             int flags,
             unsigned int offset) override;
  void Munmap(void* addr, unsigned int len) override;

  std::vector<int32_t> GetDmabufsForV4L2Buffer(
      int index,
      size_t num_planes,
      enum v4l2_buf_type buf_type) override;

  bool CanCreateEGLImageFrom(const Fourcc fourcc) const override;
  unsigned int GetTextureTarget() const override;

  SupportedProfiles GetSupportedDecodeProfiles(
      const size_t num_formats,
      const uint32_t pixelformats[]) override;
  SupportedProfiles GetSupportedEncodeProfiles() override;
  void EnumerateDevicesForType(DeviceType type) override;
  std::vector<uint32_t> PreferredInputFormat(DeviceType type) const override;

 protected:
  ~GenericV4L2Device() override;
  bool Initialize() override;

  bool OpenDevice(const std::string& path, DeviceType type);
  void CloseDevice();

  std::string GetDevicePathFor(DeviceType type, uint32_t pixfmt);
  const Devices& GetDevicesForType(DeviceType type);

  // The actual device fd.
  int32_t device_fd_ = -1;

  int32_t device_poll_interrupt_fd_ = -1;

  enum v4l2_buf_type buffer_type_ = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  std::map<DeviceType, Devices> devices_by_type_;
};

} // namespace mcil

#endif  // SRC_IMPL_V4L2_GENERIC_V4L2_DEVICE_H_
