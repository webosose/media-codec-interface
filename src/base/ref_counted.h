// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MCIL_REF_COUNTED_H_
#define BASE_MCIL_REF_COUNTED_H_

#include <cstddef>

#include <iostream>
#include <utility>

#include "scoped_refptr.h"

namespace mcil {

namespace subtle {

class RefCountedBase {
 public:
  bool HasOneRef() const { return ref_count_ == 1; }
  bool HasAtLeastOneRef() const { return ref_count_ >= 1; }

 protected:
  RefCountedBase(StartRefCountFromZeroTag) {
  }

  RefCountedBase(StartRefCountFromOneTag) : ref_count_(1) {
  }

  ~RefCountedBase() {
  }

  void AddRef() const {
    AddRefImpl();
  }

  bool Release() const {
    ReleaseImpl();
    return ref_count_ == 0;
  }

 private:
  template <typename U>
  friend scoped_refptr<U> AdoptRef(U*);

  void Adopted() const {
  }

  void AddRefImpl() const { ++ref_count_; }
  void ReleaseImpl() const { --ref_count_; }

  mutable uint32_t ref_count_ = 0;
  static_assert(std::is_unsigned<decltype(ref_count_)>::value,
                "ref_count_ must be an unsigned type.");
};

}  // namespace subtle

template <class T, typename Traits> class RefCounted;

template <typename T>
struct DefaultRefCountedTraits {
  static void Destruct(const T* x) {
    RefCounted<T, DefaultRefCountedTraits>::DeleteInternal(x);
  }
};

template <class T, typename Traits = DefaultRefCountedTraits<T> >
class RefCounted : public subtle::RefCountedBase {
 public:
  static constexpr subtle::StartRefCountFromZeroTag kRefCountPreference =
      subtle::kStartRefCountFromZeroTag;

  RefCounted() : subtle::RefCountedBase(T::kRefCountPreference) {}

  void AddRef() const {
    subtle::RefCountedBase::AddRef();
  }

  void Release() const {
    if (subtle::RefCountedBase::Release()) {
      //ANALYZER_SKIP_THIS_PATH();
      
      Traits::Destruct(static_cast<const T*>(this));
    }
  }

 protected:
  ~RefCounted() = default;

 private:
  friend struct DefaultRefCountedTraits<T>;
  template <typename U>
  static void DeleteInternal(const U* x) {
    delete x;
  }
};

}  // namespace mcil

#endif  // BASE_MCIL_REF_COUNTED_H_
