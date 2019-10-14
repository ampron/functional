#ifndef OPTION_INNER_H
#define OPTION_INNER_H

#include <fun/type_support.h>

namespace fun {

//------------------------------------------------------------------------------
template <class T, class En = void> class OptionUnion;

//------------------------------------------------------------------------------
template <class T>
class OptionUnion<T, std::enable_if_t<std::is_empty<T>::value>>: T {
  using Self = OptionUnion;

  enum class Tag: std::uint8_t { NONE, SOME };
  Tag _variant;
public:
  ~OptionUnion() = default;

  OptionUnion(const Self&) = default;

  OptionUnion(Self&& other) noexcept
    : T(static_cast<T&&>(other))
    , _variant(other._variant)
  { other._variant = Tag::NONE; }

  Self& operator=(const Self&) = default;

  Self& operator=(Self&& other) noexcept {
    if (this != &other) {
      _variant = other._variant;
      if (_variant) { static_cast<T&>(*this) = static_cast<T&&>(other); }
      other._variant = Tag::NONE;
    }
    return *this;
  }

  Self clone() const { return *this; }

  OptionUnion() : T(), _variant(Tag::NONE) {}

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

  T dump() {
    _variant = Tag::NONE;
    return static_cast<T&&>(*this);
  }

  template <class... Args>
  void emplace(Args&&... args) {
    // Constructing a temporary and throwing it away mostly replicates the
    // observable compile-time behavior of a normal emplace().
    void(T(std::forward<Args>(args)...));
    _variant = Tag::SOME;
  }
};

//------------------------------------------------------------------------------
template <class T>
class OptionUnion<T&> {
  using Self = OptionUnion<T&>;

  T* _ptr = nullptr;
public:
  ~OptionUnion() = default;

  OptionUnion(const Self&) = default;
  Self& operator=(const Self&)  = default;

  Self clone() const { return *this; }

  OptionUnion(Self&& other) noexcept: _ptr(other._ptr) {
    other._ptr = nullptr;
  }

  Self& operator=(Self&& other) noexcept {
    if (this != &other) {
      _ptr = other._ptr;
      other._ptr = nullptr;
    }
    return *this;
  }

  OptionUnion() = default;

  explicit OptionUnion(T& obj) : _ptr(std::addressof(obj)) {}

  OptionUnion(ForwardArgs, T& obj) : _ptr(std::addressof(obj)) {}

  bool is_some() const { return _ptr ? true : false; }

  T* as_ptr() { return _ptr; }

  bool operator==(const Self& other) const { return _ptr == other._ptr; }

  T& dump() {
    const auto ptr = _ptr;
    _ptr = nullptr;
    return *ptr;
  }

  void emplace(T& ref) { _ptr = std::addressof(ref); }
};

//------------------------------------------------------------------------------
template <class T, class En>
class OptionUnion {
  using Self = OptionUnion;

  enum class Tag: std::uint8_t { NONE, SOME };
  Tag _variant;

  union {
    Unit _empty = {};
    T _val;
  };

  void erase() {
    if (is_some()) {
      _variant = Tag::NONE;
      _val.~T();
      _empty = {};
    }
  }
public:
  ~OptionUnion() { erase(); }

  OptionUnion(const Self& other) : _variant(other._variant) {
    if (_variant == Tag::SOME) {
      construct_at(std::addressof(_val), other._val);
    }
  }

  Self& operator=(const Self& other) {
    if (this != &other) {
      erase();
      if (other._variant == Tag::SOME) {
        construct_at(std::addressof(_val), other._val);
        _variant = Tag::SOME;
      }
    }
    return *this;
  }

  Self clone() const { return *this; }

  OptionUnion(Self&& other) noexcept: _variant(other._variant) {
    if (_variant == Tag::SOME) {
      construct_at(std::addressof(_val), other.dump());
    }
  }

  Self& operator=(Self&& other) noexcept {
    if (this != &other) {
      erase();
      if (other._variant == Tag::SOME) {
        construct_at(std::addressof(_val), other.dump());
        _variant = Tag::SOME;
      }
    }
    return *this;
  }

  OptionUnion() : _variant(Tag::NONE) {}

  explicit OptionUnion(T val) : _variant(Tag::SOME) {
    construct_at(std::addressof(_val), std::move(val));
  }

  template <typename ...Args>
  explicit OptionUnion(ForwardArgs, Args&& ...args) : _variant(Tag::SOME) {
    construct_at(std::addressof(_val), std::forward<Args>(args)...);
  }

  bool is_some() const { return _variant == Tag::SOME; }

  T* as_ptr() { return is_some() ? std::addressof(_val) : nullptr; }

  bool operator==(const Self& other) const {
    if (is_some()) {
      return other.is_some() ? (_val == other._val) : false;
    } else {
      return !other.is_some();
    }
  }

  T dump() {
    _variant = Tag::NONE;
    auto val = std::move(_val);
    _val.~T();
    _empty = {};
#if defined(__GNUC__) && __GNUC__ <= 4
    return std::move(val);
#else
    return val;
#endif
  }

  template <typename ...Args>
  void emplace(Args&& ...args) {
    erase();
    construct_at(std::addressof(_val), std::forward<Args>(args)...);
    _variant = Tag::SOME;
  }
};

}

#endif
