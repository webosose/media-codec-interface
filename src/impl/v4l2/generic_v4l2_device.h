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
  virtual bool Open(DeviceType type, uint32_t v4l2_pixfmt) override;
  virtual int32_t Ioctl(int32_t request, void* arg) override;
  virtual bool Poll(bool poll_device, bool* event_pending) override;
  virtual bool SetDevicePollInterrupt() override;
  virtual bool ClearDevicePollInterrupt() override;
  virtual void* Mmap(void* addr, uint32_t len, int32_t prot, int32_t flags,
                     uint32_t offset) override;
  virtual void Munmap(void* addr, uint32_t len) override;

  virtual std::vector<int32_t> GetDmabufsForV4L2Buffer(
      int32_t index, size_t num_planes, enum v4l2_buf_type buffer_type)
      override;

  virtual bool CanCreateEGLImageFrom(const Fourcc fourcc) const override;
  virtual uint32_t GetTextureTarget() const override;

  virtual SupportedProfiles GetSupportedDecodeProfiles() override;
  virtual SupportedProfiles GetSupportedEncodeProfiles() override;
  virtual void EnumerateDevicesForType(DeviceType type) override;
  virtual std::vector<uint32_t> PreferredInputFormat(DeviceType type)
      const override;

 protected:
  virtual ~GenericV4L2Device() noexcept(false);
  virtual bool Initialize() override;

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
