#ifndef OPTION_INNER_H
#define OPTION_INNER_H

#include <fun/type_support.h>

namespace fun {

//------------------------------------------------------------------------------
template <class T, class En = void> struct OptionUnion;

//------------------------------------------------------------------------------
template <class T>
struct OptionUnion<T, std::enable_if_t<std::is_empty<T>::value>>: private T {
  using Self = OptionUnion;

  enum class Tag: std::uint8_t { NONE, SOME };
  Tag _variant;

  ~OptionUnion() = default;

  OptionUnion(const Self&) = default;

  OptionUnion(Self&& other) noexcept
    : T(static_cast<T&&>(other))
    , _variant(other._variant)
  { other._variant = Tag::NONE; }

  Self& operator=(const Self&) = default;

  Self& operator=(Self&& other) noexcept {
    if (this != &other) {
      static_cast<T&>(*this) = static_cast<T&&>(other);
      _variant = other._variant;
      other._variant = Tag::NONE;
    }
  }

  Self clone() const { return *this; }

  OptionUnion() : _variant(Tag::NONE) {}

  explicit OptionUnion(T val) : T(std::move(val)), _variant(Tag::SOME) {}

  template <typename ...Args>
  explicit OptionUnion(ForwardArgs, Args&& ...args)
    : T(std::forward<Args>(args)...)
    , _variant(Tag::SOME)
  {}

  bool is_some() const { return _variant == Tag::SOME; }

  T* as_ptr() { return is_some() ? static_cast<T*>(this) : nullptr; }

  bool operator==(const Self& other) const {
    if (_variant != other._variant) { return false; }
    if (is_some()) {
      return static_cast<T const&>(*this) == static_cast<T const&>(other);
    }
    return true;
  }

  T unwrap() { return static_cast<T&&>(*this); }

  template <class... Args>
  void emplace(Args&&... args) {
    static_cast<T&>(*this) = T(std::forward<Args>(args)...);
    _variant = Tag::SOME;
  }

};

//------------------------------------------------------------------------------
template <class T>
struct OptionUnion<T&> {
  using Self = OptionUnion<T&>;

  T* _ptr = nullptr;

  ~OptionUnion() = default;

  OptionUnion(const Self&) = default;
  Self& operator=(const Self&)  = default;

  Self clone() const { return Self(*this); }

  // For reference type move = copy
  OptionUnion(Self&& other) = default;
  Self& operator=(Self&&) = default;

  OptionUnion() = default;

  explicit OptionUnion(T& obj) : _ptr(&obj) {}
  explicit OptionUnion(T* ptr) : _ptr(ptr) {}

  OptionUnion(ForwardArgs, T& obj) : _ptr(&obj) {}

  bool is_some() const { return _ptr ? true : false; }

  T* as_ptr() { return _ptr; }

  bool operator==(const Self& other) const { return _ptr == other._ptr; }

  T& unwrap() { return *_ptr; }

  void emplace(T& ref) { _ptr = std::addressof(ref); }
};

//------------------------------------------------------------------------------
template <class T, class En>
struct OptionUnion {
  using Self = OptionUnion;

  enum class Tag: std::uint8_t { NONE, SOME };
  Tag _variant;
  union {
    Unit _empty = {};
    std::remove_const_t<T> _val;
  };

  ~OptionUnion() {
    if (_variant == Tag::SOME) { _val.~T(); }
  }

  OptionUnion(const Self& other) : _variant(other._variant) {
    if (_variant == Tag::SOME) { new (&_val) T(other._val); }
  }

  Self& operator=(const Self& other) {
    if (this != &other) {
      if (_variant == Tag::SOME) {
        _variant = Tag::NONE;
        _val.~T();
        _empty = {};
      }
      if (other._variant == Tag::SOME) {
        new (&_val) T(other._val);
        _variant = Tag::SOME;
      }
    }
    return *this;
  }

  Self clone() const { return *this; }

  OptionUnion(Self&& other) noexcept: _variant(other._variant) {
    if (_variant == Tag::SOME) { new (&_val) T(std::move(other._val)); }
    other._variant = Tag::NONE;
  }

  Self& operator=(Self&& other) noexcept {
    if (this != &other) {
      if (_variant == Tag::SOME) { _val.~T(); }
      _variant = other._variant;
      if (_variant == Tag::SOME) { new (&_val) T(std::move(other._val)); }
      other._variant = Tag::NONE;
    }
    return *this;
  }

  OptionUnion() : _variant(Tag::NONE) {}

  explicit OptionUnion(T val) : _variant(Tag::SOME), _val(std::move(val)) {}

  template <typename ...Args>
  explicit OptionUnion(ForwardArgs, Args&& ...args)
    : _variant(Tag::SOME), _val(std::forward<Args>(args)...)
  {}

  bool is_some() const { return _variant == Tag::SOME; }

  T* as_ptr() { return is_some() ? &_val : nullptr; }

  bool operator==(const Self& other) const {
    if (is_some()) {
      return other.is_some() ? (_val == other._val) : false;
    } else {
      return !other.is_some();
    }
  }

  T unwrap() { return std::move(_val); }

  template <typename ...Args>
  void emplace(Args&& ...args) {
    if (_variant == Tag::SOME) {
      _variant = Tag::NONE;
      _val.~T();
      _empty = {};
    }
    new (&_val) T(std::forward<Args>(args)...);
    _variant = Tag::SOME;
  }
};

}

#endif
