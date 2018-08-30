#ifndef OPTION_INNER_H
#define OPTION_INNER_H

#include <fun/type_support.h>

namespace fun {

//------------------------------------------------------------------------------
template <class T> struct OptionUnion;

//------------------------------------------------------------------------------
template <>
struct OptionUnion<Unit> {
  using Self = OptionUnion<Unit>;
  
  enum { NONE, SOME } _variant;
  Unit _val;
  
  ~OptionUnion() = default;
  
  OptionUnion(const Self&) = default;
  Self& operator=(const Self&)  = default;
  
  Self clone() const { return Self(*this); }
  
  // For reference type move = copy
  OptionUnion(Self&& other) = default;
  Self& operator=(Self&&) = default;
  
  OptionUnion() : _variant(NONE) {}

  explicit OptionUnion(Unit) : _variant(SOME) {}

  template <typename ...Args>
  explicit OptionUnion(ForwardArgs, Args&& ...args) : _variant(SOME) {}
  
  
  bool is_some() const { return _variant == SOME; }
  
  Unit* as_ptr() { return is_some() ? &_val : nullptr; }
  
  bool operator==(const Self& other) const { return _variant == other._variant; }
  
  Unit& unwrap() { return _val; }
  
  void emplace() {}
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
  
  void emplace(T* ptr) { _ptr = ptr; }
};

//------------------------------------------------------------------------------
template <class T>
struct OptionUnion {
  using Self = OptionUnion<T>;
  
  enum { NONE, SOME } _variant;
  union {
    uint8_t _empty;
    T _val;
  };
  
  // ** only call on SOME variant, otherwise undefined behavior **
  T dump() {
  	auto temp = std::move(_val);
  	_val.~T();
  	_empty   = 0;
  	_variant = NONE;
  	return temp;
  }
  
  ~OptionUnion() {
    if (_variant == SOME) { _val.~T(); }
    _variant = NONE;
  }
  
  OptionUnion(const Self& other) : OptionUnion() { *this = other; }
  Self& operator=(const Self& other) {
    if (this != &other) {
      _variant = other._variant;
      if (_variant == NONE) { _empty = 0; }
      else { new (&_val) T(other._val); }
    }
    return *this;
  }
  
  Self clone() const { return Self(*this); }
  
  OptionUnion(Self&& other) : OptionUnion() { *this = std::move(other); }
  Self& operator=(Self&& other) {
    if (this != &other) {
      // clear out this option if it contatins something
      if (_variant == SOME) { dump(); }
      
      // move contents of other option into this one
      _variant = other._variant;
      if (_variant == NONE) { _empty = 0; }
      else { new (&_val) T(other.dump()); }
    }
    return *this;
  }
  
  OptionUnion() : _variant(NONE), _empty(0) {}

  explicit OptionUnion(T val) : _variant(SOME), _val(std::move(val)) {}
  
  template <typename ...Args>
  explicit OptionUnion(ForwardArgs, Args&& ...args)
    : _variant(SOME), _val(std::forward<Args>(args)...)
  {}
  
  bool is_some() const { return _variant == SOME; }
  
  T* as_ptr() { return is_some() ? &_val : nullptr; }
  
  bool operator==(const Self& other) const {
    if (is_some()) {
      return other.is_some() ? (_val == other._val) : false;
    } else {
      return !other.is_some();
    }
  }
  
  T unwrap() { return dump(); }

  template <typename ...Args>
  void emplace(Args&& ...args) {
    if (is_some()) { dump(); }
    _variant = SOME;
    new (&_val) T(std::forward<Args>(args) ...);
  }
};

}

#endif
