// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MCIL_OPTIONAL_H_
#define BASE_MCIL_OPTIONAL_H_

#include <functional>
#include <type_traits>
#include <utility>

namespace mcil {

struct nullopt_t {
  constexpr explicit nullopt_t(int) {}
#if !defined(__clang__)
  nullopt_t() {}
  operator int() const { return 0; }
#endif
};

constexpr nullopt_t nullopt(0);

template <typename T>
class Optional;

namespace internal {

#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ <= 7
template <typename T>
struct is_trivially_copy_constructible
    : std::is_trivially_copy_constructible<T> {};

template <typename... T>
struct is_trivially_copy_constructible<std::vector<T...>> : std::false_type {};

template <typename T>
struct is_trivially_move_constructible
    : std::is_trivially_move_constructible<T> {};

template <typename... T>
struct is_trivially_move_constructible<std::vector<T...>> : std::false_type {};
#else
template <typename T>
using is_trivially_copy_constructible = std::is_trivially_copy_constructible<T>;
template <typename T>
using is_trivially_move_constructible = std::is_trivially_move_constructible<T>;
#endif

struct in_place_t {};
constexpr in_place_t in_place = {};

template <typename T>
struct in_place_type_t {};

template <typename T>
struct is_in_place_type_t {
  static constexpr bool value = false;
};

template <typename... Ts>
struct is_in_place_type_t<in_place_type_t<Ts...>> {
  static constexpr bool value = true;
};

template <typename...>
struct disjunction : std::false_type {};

template <typename B1>
struct disjunction<B1> : B1 {};

template <typename B1, typename... Bn>
struct disjunction<B1, Bn...>
    : std::conditional_t<static_cast<bool>(B1::value), B1, disjunction<Bn...>> {
};

struct DummyUnionMember {};

template <typename T, bool = std::is_trivially_destructible<T>::value>
struct OptionalStorageBase {
  constexpr OptionalStorageBase() : dummy_() {}

  template <class... Args>
  constexpr explicit OptionalStorageBase(in_place_t, Args&&... args)
      : is_populated_(true), value_(std::forward<Args>(args)...) {}

  ~OptionalStorageBase() {
    if (is_populated_)
      value_.~T();
  }

  template <class... Args>
  void Init(Args&&... args) {
    ::new (std::addressof(value_)) T(std::forward<Args>(args)...);
    is_populated_ = true;
  }

  bool is_populated_ = false;
  union {
    DummyUnionMember dummy_;
    T value_;
  };
};

template <typename T>
struct OptionalStorageBase<T, true /* trivially destructible */> {
  constexpr OptionalStorageBase() : dummy_() {}

  template <class... Args>
  constexpr explicit OptionalStorageBase(in_place_t, Args&&... args)
      : is_populated_(true), value_(std::forward<Args>(args)...) {}

  template <class... Args>
  void Init(Args&&... args) {
    ::new (std::addressof(value_)) T(std::forward<Args>(args)...);
    is_populated_ = true;
  }

  bool is_populated_ = false;
  union {
    DummyUnionMember dummy_;
    T value_;
  };
};

template <typename T,
          bool = is_trivially_copy_constructible<T>::value,
          bool = is_trivially_move_constructible<T>::value>
struct OptionalStorage : OptionalStorageBase<T> {
  using OptionalStorageBase<T>::is_populated_;
  using OptionalStorageBase<T>::value_;
  using OptionalStorageBase<T>::Init;

  using OptionalStorageBase<T>::OptionalStorageBase;

  OptionalStorage() = default;

  OptionalStorage(const OptionalStorage& other) {
    if (other.is_populated_)
      Init(other.value_);
  }

  OptionalStorage(OptionalStorage&& other) noexcept(
      std::is_nothrow_move_constructible<T>::value) {
    if (other.is_populated_)
      Init(std::move(other.value_));
  }
};

template <typename T>
struct OptionalStorage<T,
                       true /* trivially copy constructible */,
                       false /* trivially move constructible */>
    : OptionalStorageBase<T> {
  using OptionalStorageBase<T>::is_populated_;
  using OptionalStorageBase<T>::value_;
  using OptionalStorageBase<T>::Init;
  using OptionalStorageBase<T>::OptionalStorageBase;

  OptionalStorage() = default;
  OptionalStorage(const OptionalStorage& other) = default;

  OptionalStorage(OptionalStorage&& other) noexcept(
      std::is_nothrow_move_constructible<T>::value) {
    if (other.is_populated_)
      Init(std::move(other.value_));
  }
};

template <typename T>
struct OptionalStorage<T,
                       false /* trivially copy constructible */,
                       true /* trivially move constructible */>
    : OptionalStorageBase<T> {
  using OptionalStorageBase<T>::is_populated_;
  using OptionalStorageBase<T>::value_;
  using OptionalStorageBase<T>::Init;
  using OptionalStorageBase<T>::OptionalStorageBase;

  OptionalStorage() = default;
  OptionalStorage(OptionalStorage&& other) = default;

  OptionalStorage(const OptionalStorage& other) {
    if (other.is_populated_)
      Init(other.value_);
  }
};

template <typename T>
struct OptionalStorage<T,
                       true /* trivially copy constructible */,
                       true /* trivially move constructible */>
    : OptionalStorageBase<T> {
  using OptionalStorageBase<T>::OptionalStorageBase;
};

template <typename T>
class OptionalBase {
 protected:
  constexpr OptionalBase() = default;
  constexpr OptionalBase(const OptionalBase& other) = default;
  constexpr OptionalBase(OptionalBase&& other) = default;

  template <class... Args>
  constexpr explicit OptionalBase(in_place_t, Args&&... args)
      : storage_(in_place, std::forward<Args>(args)...) {}

  template <typename U>
  explicit OptionalBase(const OptionalBase<U>& other) {
    if (other.storage_.is_populated_)
      storage_.Init(other.storage_.value_);
  }

  template <typename U>
  explicit OptionalBase(OptionalBase<U>&& other) {
    if (other.storage_.is_populated_)
      storage_.Init(std::move(other.storage_.value_));
  }

  ~OptionalBase() = default;

  OptionalBase& operator=(const OptionalBase& other) {
    CopyAssign(other);
    return *this;
  }

  OptionalBase& operator=(OptionalBase&& other) noexcept(
      std::is_nothrow_move_assignable<T>::value&&
          std::is_nothrow_move_constructible<T>::value) {
    MoveAssign(std::move(other));
    return *this;
  }

  template <typename U>
  void CopyAssign(const OptionalBase<U>& other) {
    if (other.storage_.is_populated_)
      InitOrAssign(other.storage_.value_);
    else
      FreeIfNeeded();
  }

  template <typename U>
  void MoveAssign(OptionalBase<U>&& other) {
    if (other.storage_.is_populated_)
      InitOrAssign(std::move(other.storage_.value_));
    else
      FreeIfNeeded();
  }

  template <typename U>
  void InitOrAssign(U&& value) {
    if (storage_.is_populated_)
      storage_.value_ = std::forward<U>(value);
    else
      storage_.Init(std::forward<U>(value));
  }

  void FreeIfNeeded() {
    if (!storage_.is_populated_)
      return;
    storage_.value_.~T();
    storage_.is_populated_ = false;
  }

  template <typename U>
  friend class OptionalBase;

  OptionalStorage<T> storage_;
};

template <bool is_copy_constructible>
struct CopyConstructible {};

template <>
struct CopyConstructible<false> {
  constexpr CopyConstructible() = default;
  constexpr CopyConstructible(const CopyConstructible&) = delete;
  constexpr CopyConstructible(CopyConstructible&&) = default;
  CopyConstructible& operator=(const CopyConstructible&) = default;
  CopyConstructible& operator=(CopyConstructible&&) = default;
};

template <bool is_move_constructible>
struct MoveConstructible {};

template <>
struct MoveConstructible<false> {
  constexpr MoveConstructible() = default;
  constexpr MoveConstructible(const MoveConstructible&) = default;
  constexpr MoveConstructible(MoveConstructible&&) = delete;
  MoveConstructible& operator=(const MoveConstructible&) = default;
  MoveConstructible& operator=(MoveConstructible&&) = default;
};

template <bool is_copy_assignable>
struct CopyAssignable {};

template <>
struct CopyAssignable<false> {
  constexpr CopyAssignable() = default;
  constexpr CopyAssignable(const CopyAssignable&) = default;
  constexpr CopyAssignable(CopyAssignable&&) = default;
  CopyAssignable& operator=(const CopyAssignable&) = delete;
  CopyAssignable& operator=(CopyAssignable&&) = default;
};

template <bool is_move_assignable>
struct MoveAssignable {};

template <>
struct MoveAssignable<false> {
  constexpr MoveAssignable() = default;
  constexpr MoveAssignable(const MoveAssignable&) = default;
  constexpr MoveAssignable(MoveAssignable&&) = default;
  MoveAssignable& operator=(const MoveAssignable&) = default;
  MoveAssignable& operator=(MoveAssignable&&) = delete;
};

template <typename T, typename U>
using IsConvertibleFromOptional =
    disjunction<std::is_constructible<T, Optional<U>&>,
                std::is_constructible<T, const Optional<U>&>,
                std::is_constructible<T, Optional<U>&&>,
                std::is_constructible<T, const Optional<U>&&>,
                std::is_convertible<Optional<U>&, T>,
                std::is_convertible<const Optional<U>&, T>,
                std::is_convertible<Optional<U>&&, T>,
                std::is_convertible<const Optional<U>&&, T>>;

template <typename T, typename U>
using IsAssignableFromOptional =
    disjunction<IsConvertibleFromOptional<T, U>,
                std::is_assignable<T&, Optional<U>&>,
                std::is_assignable<T&, const Optional<U>&>,
                std::is_assignable<T&, Optional<U>&&>,
                std::is_assignable<T&, const Optional<U>&&>>;

namespace swappable_impl {
using std::swap;

struct IsSwappableImpl {
  template <typename T>
  static auto Check(int)
      -> decltype(swap(std::declval<T>(), std::declval<T>()), std::true_type());

  template <typename T>
  static std::false_type Check(...);
};
}  // namespace swappable_impl

template <typename T>
struct IsSwappable : decltype(swappable_impl::IsSwappableImpl::Check<T&>(0)) {};

template <typename T>
using RemoveCvRefT = std::remove_cv_t<std::remove_reference_t<T>>;

}  // namespace internal

#define OPTIONAL_DECLSPEC_EMPTY_BASES

template <typename T>
class OPTIONAL_DECLSPEC_EMPTY_BASES Optional
    : public internal::OptionalBase<T>,
      public internal::CopyConstructible<std::is_copy_constructible<T>::value>,
      public internal::MoveConstructible<std::is_move_constructible<T>::value>,
      public internal::CopyAssignable<std::is_copy_constructible<T>::value &&
                                      std::is_copy_assignable<T>::value>,
      public internal::MoveAssignable<std::is_move_constructible<T>::value &&
                                      std::is_move_assignable<T>::value> {
 private:

  static_assert(
      !std::is_same<internal::RemoveCvRefT<T>, internal::in_place_t>::value,
      "instantiation of Optional with in_place_t is ill-formed");
  static_assert(!std::is_same<internal::RemoveCvRefT<T>, nullopt_t>::value,
                "instantiation of Optional with nullopt_t is ill-formed");
  static_assert(
      !std::is_reference<T>::value,
      "instantiation of Optional with a reference type is ill-formed");

  static_assert(std::is_destructible<T>::value,
                "instantiation of Optional with a non-destructible type "
                "is ill-formed");

  static_assert(
      !std::is_array<T>::value,
      "instantiation of Optional with an array type is ill-formed");

 public:
#undef OPTIONAL_DECLSPEC_EMPTY_BASES
  using value_type = T;

  constexpr Optional() = default;
  constexpr Optional(const Optional& other) = default;
  constexpr Optional(Optional&& other) noexcept(
      std::is_nothrow_move_constructible<T>::value) = default;

  constexpr Optional(nullopt_t) {}  // NOLINT(runtime/explicit)

  template <
      typename U,
      std::enable_if_t<std::is_constructible<T, const U&>::value &&
                           !internal::IsConvertibleFromOptional<T, U>::value &&
                           std::is_convertible<const U&, T>::value,
                       bool> = false>
  Optional(const Optional<U>& other) : internal::OptionalBase<T>(other) {}

  template <
      typename U,
      std::enable_if_t<std::is_constructible<T, const U&>::value &&
                           !internal::IsConvertibleFromOptional<T, U>::value &&
                           !std::is_convertible<const U&, T>::value,
                       bool> = false>
  explicit Optional(const Optional<U>& other)
      : internal::OptionalBase<T>(other) {}

  template <
      typename U,
      std::enable_if_t<std::is_constructible<T, U&&>::value &&
                           !internal::IsConvertibleFromOptional<T, U>::value &&
                           std::is_convertible<U&&, T>::value,
                       bool> = false>
  Optional(Optional<U>&& other) : internal::OptionalBase<T>(std::move(other)) {}

  template <
      typename U,
      std::enable_if_t<std::is_constructible<T, U&&>::value &&
                           !internal::IsConvertibleFromOptional<T, U>::value &&
                           !std::is_convertible<U&&, T>::value,
                       bool> = false>
  explicit Optional(Optional<U>&& other)
      : internal::OptionalBase<T>(std::move(other)) {}

  template <class... Args>
  constexpr explicit Optional(internal::in_place_t, Args&&... args)
      : internal::OptionalBase<T>(internal::in_place,
                                  std::forward<Args>(args)...) {}

  template <
      class U,
      class... Args,
      class = std::enable_if_t<std::is_constructible<value_type,
                                                     std::initializer_list<U>&,
                                                     Args...>::value>>
  constexpr explicit Optional(internal::in_place_t,
                              std::initializer_list<U> il,
                              Args&&... args)
      : internal::OptionalBase<T>(internal::in_place, il,
                                  std::forward<Args>(args)...) {}

  template <
      typename U = value_type,
      std::enable_if_t<
          std::is_constructible<T, U&&>::value &&
              !std::is_same<internal::RemoveCvRefT<U>,
                            internal::in_place_t>::value &&
              !std::is_same<internal::RemoveCvRefT<U>, Optional<T>>::value &&
              std::is_convertible<U&&, T>::value,
          bool> = false>
  constexpr Optional(U&& value)
      : internal::OptionalBase<T>(internal::in_place, std::forward<U>(value)) {}

  template <
      typename U = value_type,
      std::enable_if_t<
          std::is_constructible<T, U&&>::value &&
              !std::is_same<internal::RemoveCvRefT<U>,
                            internal::in_place_t>::value &&
              !std::is_same<internal::RemoveCvRefT<U>, Optional<T>>::value &&
              !std::is_convertible<U&&, T>::value,
          bool> = false>
  constexpr explicit Optional(U&& value)
      : internal::OptionalBase<T>(internal::in_place, std::forward<U>(value)) {}

  ~Optional() = default;

  Optional& operator=(const Optional& other) = default;
  Optional& operator=(Optional&& other) noexcept(
      std::is_nothrow_move_assignable<T>::value&&
          std::is_nothrow_move_constructible<T>::value) = default;

  Optional& operator=(nullopt_t) {
    FreeIfNeeded();
    return *this;
  }

  // Perfect-forwarded assignment.
  template <typename U>
  std::enable_if_t<
      !std::is_same<internal::RemoveCvRefT<U>, Optional<T>>::value &&
          std::is_constructible<T, U>::value &&
          std::is_assignable<T&, U>::value &&
          (!std::is_scalar<T>::value ||
           !std::is_same<std::decay_t<U>, T>::value),
      Optional&>
  operator=(U&& value) {
    InitOrAssign(std::forward<U>(value));
    return *this;
  }

  // Copy assign the state of other.
  template <typename U>
  std::enable_if_t<!internal::IsAssignableFromOptional<T, U>::value &&
                       std::is_constructible<T, const U&>::value &&
                       std::is_assignable<T&, const U&>::value,
                   Optional&>
  operator=(const Optional<U>& other) {
    CopyAssign(other);
    return *this;
  }

  // Move assign the state of other.
  template <typename U>
  std::enable_if_t<!internal::IsAssignableFromOptional<T, U>::value &&
                       std::is_constructible<T, U>::value &&
                       std::is_assignable<T&, U>::value,
                   Optional&>
  operator=(Optional<U>&& other) {
    MoveAssign(std::move(other));
    return *this;
  }

  constexpr const T* operator->() const {
    return std::addressof(storage_.value_);
  }

  constexpr T* operator->() {
    return std::addressof(storage_.value_);
  }

  constexpr const T& operator*() const & {
    return storage_.value_;
  }

  constexpr T& operator*() & {
    return storage_.value_;
  }

  constexpr const T&& operator*() const && {
    return std::move(storage_.value_);
  }

  constexpr T&& operator*() && {
    return std::move(storage_.value_);
  }

  constexpr explicit operator bool() const { return storage_.is_populated_; }

  constexpr bool has_value() const { return storage_.is_populated_; }

  constexpr T& value() & {
    return storage_.value_;
  }

  constexpr const T& value() const & {
    return storage_.value_;
  }

  constexpr T&& value() && {
    return std::move(storage_.value_);
  }

  constexpr const T&& value() const && {
    return std::move(storage_.value_);
  }

  template <class U>
  constexpr T value_or(U&& default_value) const& {
    static_assert(std::is_convertible<U, T>::value,
                  "U must be convertible to T");
    return storage_.is_populated_
               ? storage_.value_
               : static_cast<T>(std::forward<U>(default_value));
  }

  template <class U>
  constexpr T value_or(U&& default_value) && {
    static_assert(std::is_convertible<U, T>::value,
                  "U must be convertible to T");
    return storage_.is_populated_
               ? std::move(storage_.value_)
               : static_cast<T>(std::forward<U>(default_value));
  }

  void swap(Optional& other) {
    if (!storage_.is_populated_ && !other.storage_.is_populated_)
      return;

    if (storage_.is_populated_ != other.storage_.is_populated_) {
      if (storage_.is_populated_) {
        other.storage_.Init(std::move(storage_.value_));
        FreeIfNeeded();
      } else {
        storage_.Init(std::move(other.storage_.value_));
        other.FreeIfNeeded();
      }
      return;
    }

    using std::swap;
    swap(**this, *other);
  }

  void reset() { FreeIfNeeded(); }

  template <class... Args>
  T& emplace(Args&&... args) {
    FreeIfNeeded();
    storage_.Init(std::forward<Args>(args)...);
    return storage_.value_;
  }

  template <class U, class... Args>
  std::enable_if_t<
      std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value,
      T&>
  emplace(std::initializer_list<U> il, Args&&... args) {
    FreeIfNeeded();
    storage_.Init(il, std::forward<Args>(args)...);
    return storage_.value_;
  }

 private:
  using internal::OptionalBase<T>::CopyAssign;
  using internal::OptionalBase<T>::FreeIfNeeded;
  using internal::OptionalBase<T>::InitOrAssign;
  using internal::OptionalBase<T>::MoveAssign;
  using internal::OptionalBase<T>::storage_;
};

template <class T, class U>
constexpr bool operator==(const Optional<T>& lhs, const Optional<U>& rhs) {
  if (lhs.has_value() != rhs.has_value())
    return false;
  if (!lhs.has_value())
    return true;
  return *lhs == *rhs;
}

template <class T, class U>
constexpr bool operator!=(const Optional<T>& lhs, const Optional<U>& rhs) {
  if (lhs.has_value() != rhs.has_value())
    return true;
  if (!lhs.has_value())
    return false;
  return *lhs != *rhs;
}

template <class T, class U>
constexpr bool operator<(const Optional<T>& lhs, const Optional<U>& rhs) {
  if (!rhs.has_value())
    return false;
  if (!lhs.has_value())
    return true;
  return *lhs < *rhs;
}

template <class T, class U>
constexpr bool operator<=(const Optional<T>& lhs, const Optional<U>& rhs) {
  if (!lhs.has_value())
    return true;
  if (!rhs.has_value())
    return false;
  return *lhs <= *rhs;
}

template <class T, class U>
constexpr bool operator>(const Optional<T>& lhs, const Optional<U>& rhs) {
  if (!lhs.has_value())
    return false;
  if (!rhs.has_value())
    return true;
  return *lhs > *rhs;
}

template <class T, class U>
constexpr bool operator>=(const Optional<T>& lhs, const Optional<U>& rhs) {
  if (!rhs.has_value())
    return true;
  if (!lhs.has_value())
    return false;
  return *lhs >= *rhs;
}

template <class T>
constexpr bool operator==(const Optional<T>& opt, nullopt_t) {
  return !opt;
}

template <class T>
constexpr bool operator==(nullopt_t, const Optional<T>& opt) {
  return !opt;
}

template <class T>
constexpr bool operator!=(const Optional<T>& opt, nullopt_t) {
  return opt.has_value();
}

template <class T>
constexpr bool operator!=(nullopt_t, const Optional<T>& opt) {
  return opt.has_value();
}

template <class T>
constexpr bool operator<(const Optional<T>& opt, nullopt_t) {
  return false;
}

template <class T>
constexpr bool operator<(nullopt_t, const Optional<T>& opt) {
  return opt.has_value();
}

template <class T>
constexpr bool operator<=(const Optional<T>& opt, nullopt_t) {
  return !opt;
}

template <class T>
constexpr bool operator<=(nullopt_t, const Optional<T>& opt) {
  return true;
}

template <class T>
constexpr bool operator>(const Optional<T>& opt, nullopt_t) {
  return opt.has_value();
}

template <class T>
constexpr bool operator>(nullopt_t, const Optional<T>& opt) {
  return false;
}

template <class T>
constexpr bool operator>=(const Optional<T>& opt, nullopt_t) {
  return true;
}

template <class T>
constexpr bool operator>=(nullopt_t, const Optional<T>& opt) {
  return !opt;
}

template <class T, class U>
constexpr bool operator==(const Optional<T>& opt, const U& value) {
  return opt.has_value() ? *opt == value : false;
}

template <class T, class U>
constexpr bool operator==(const U& value, const Optional<T>& opt) {
  return opt.has_value() ? value == *opt : false;
}

template <class T, class U>
constexpr bool operator!=(const Optional<T>& opt, const U& value) {
  return opt.has_value() ? *opt != value : true;
}

template <class T, class U>
constexpr bool operator!=(const U& value, const Optional<T>& opt) {
  return opt.has_value() ? value != *opt : true;
}

template <class T, class U>
constexpr bool operator<(const Optional<T>& opt, const U& value) {
  return opt.has_value() ? *opt < value : true;
}

template <class T, class U>
constexpr bool operator<(const U& value, const Optional<T>& opt) {
  return opt.has_value() ? value < *opt : false;
}

template <class T, class U>
constexpr bool operator<=(const Optional<T>& opt, const U& value) {
  return opt.has_value() ? *opt <= value : true;
}

template <class T, class U>
constexpr bool operator<=(const U& value, const Optional<T>& opt) {
  return opt.has_value() ? value <= *opt : false;
}

template <class T, class U>
constexpr bool operator>(const Optional<T>& opt, const U& value) {
  return opt.has_value() ? *opt > value : false;
}

template <class T, class U>
constexpr bool operator>(const U& value, const Optional<T>& opt) {
  return opt.has_value() ? value > *opt : true;
}

template <class T, class U>
constexpr bool operator>=(const Optional<T>& opt, const U& value) {
  return opt.has_value() ? *opt >= value : false;
}

template <class T, class U>
constexpr bool operator>=(const U& value, const Optional<T>& opt) {
  return opt.has_value() ? value >= *opt : true;
}

template <class T>
constexpr Optional<std::decay_t<T>> make_optional(T&& value) {
  return Optional<std::decay_t<T>>(std::forward<T>(value));
}

template <class T, class... Args>
constexpr Optional<T> make_optional(Args&&... args) {
  return Optional<T>(internal::in_place, std::forward<Args>(args)...);
}

template <class T, class U, class... Args>
constexpr Optional<T> make_optional(std::initializer_list<U> il,
                                    Args&&... args) {
  return Optional<T>(internal::in_place, il, std::forward<Args>(args)...);
}

template <class T>
std::enable_if_t<std::is_move_constructible<T>::value &&
                 internal::IsSwappable<T>::value>
swap(Optional<T>& lhs, Optional<T>& rhs) {
  lhs.swap(rhs);
}

}  // namespace mcil

namespace std {

template <class T>
struct hash<mcil::Optional<T>> {
  size_t operator()(const mcil::Optional<T>& opt) const {
    return opt == mcil::nullopt ? 0 : std::hash<T>()(*opt);
  }
};

}  // namespace std

#endif  // BASE_MCIL_OPTIONAL_H_
