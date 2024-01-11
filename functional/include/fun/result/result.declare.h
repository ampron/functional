#pragma once

//!
//! @author Alex Pronschinske
//! @copyright MIT LIcense
//!

#include <ostream>

#include <fun/type_support.h>
#include <fun/option/option.declare.h>

namespace fun {

//------------------------------------------------------------------------------
template <class T, class E> class Result;

template <class T> struct MakeOkResult{ T val; };
template <class E> struct MakeErrResult{ E val; };

template <class T>
auto ok(T val) -> MakeOkResult<T>;

template <class E, class T>
auto ok(T val) -> Result<T, E>;

template <class T>
auto ok_cref(const T& val) -> MakeOkResult<const T&>;
template <class T>
auto ok_ref(T& val) -> MakeOkResult<T&>;

template <class E, class T>
auto ok_ref(T& val) -> Result<T&, E>;

template <class E>
auto err(E val) -> MakeErrResult<E>;

template <class T, class E>
auto err(E val) -> Result<T, E>;

template <class E>
auto err_cref(const E& val) -> MakeErrResult<const E&>;
template <class E>
auto err_ref(E& val) -> MakeErrResult<E&>;

template <class T, class E>
auto err_ref(E& val) -> Result<T, E&>;

struct OkTag {};
struct ErrTag {};

template <class Tag, class ...Args>
struct MakeResultArgs { std::tuple<Args...> tup; };

template <class ...Args>
auto make_ok(Args&& ...args) -> MakeResultArgs<OkTag, Args&&...>
{
  return { std::forward_as_tuple(std::forward<Args>(args)...) };
}

template <class ...Args>
auto make_err(Args&& ...args) -> MakeResultArgs<ErrTag, Args&&...>
{
  return { std::forward_as_tuple(std::forward<Args>(args)...) };
}

//------------------------------------------------------------------------------
template <class T, class E>
class Result {
public:
  using self_t = Result<T, E>;

  using value_t = std::remove_reference_t<T>;
  using error_t = std::remove_reference_t<E>;

private:
  enum { Ok, Err } _variant;
  union {
    Sized<T> _ok;
    Sized<E> _err;
  };

  // ** only call on `Ok` variant, otherwise undefined behavior **
  T dump_ok();

  // ** only call on `Err` variant, otherwise undefined behavior **
  E dump_err();

public:
  ~Result();

  Result(self_t&&);
  auto operator=(self_t&&) -> self_t&;

  Result(const self_t&);
  auto operator=(const self_t&) -> self_t&;

  Result() = delete;

  auto clone() const -> self_t;

  template <typename ...Args>
  Result(OkTag, ForwardArgs, Args&& ...args);

  template <typename ...Args>
  Result(ErrTag, ForwardArgs, Args&& ...args);

  Result(MakeOkResult<T>);
  Result(MakeErrResult<E>);

  template <class Tag, class ...Args, size_t ...Indices>
  Result(Tag tag, std::tuple<Args...>& args, std::integer_sequence<size_t, Indices...>)
    : Result(tag, ForwardArgs{}, std::forward<Args>(std::get<Indices>(args))...)
  {}

  template <class Tag, class ...Args>
  Result(MakeResultArgs<Tag, Args...>&& make_args)
    : Result(Tag{}, make_args.tup, std::index_sequence_for<Args...>{})
  {}

  auto operator=(const MakeOkResult<T>&) -> self_t&;
  auto operator=(const MakeErrResult<E>&) -> self_t&;

  bool is_ok() const;
  bool is_err() const;
  explicit operator bool() const { return is_ok(); }

  auto as_ptr() -> value_t*;
  auto as_ptr() const -> const value_t*;
  auto as_const_ptr() const -> const value_t* { return as_ptr(); }
  auto as_err_ptr() -> error_t*;
  auto as_err_ptr() const -> const error_t*;
  auto as_const_err_ptr() const -> const error_t* { return as_err_ptr(); }

  bool operator==(const self_t& other) const;
  bool operator!=(const self_t& other) const;

  bool operator==(const MakeOkResult<T>& other) const {
    if (is_ok()) { return _ok._val == other.val; }
    else         { return false; }
  }
  bool operator==(const MakeErrResult<E>& other) const {
    if (is_ok()) { return false; }
    else         { return _err._val == other.val; }
  }

  template <class U>
  bool operator!=(const U& other) const { return !(*this == other); }

  auto unwrap() && -> T;

  auto unwrap_or(T alt) && -> T;

  template <class F>
  auto unwrap_or_else(F alt_func) && -> T;

  auto unwrap_or_default() && -> T {
    return std::move(*this).unwrap_or_else([](auto&&) -> T { return {}; });
  }

  auto unwrap_err() && -> E;

  auto as_ref() -> Result<value_t&, error_t&>;

  auto as_ref() const -> Result<const value_t&, const error_t&>;

  auto as_cref() const -> Result<const value_t&, const error_t&>;

  auto ok() && -> Option<T>;
  auto err() && -> Option<E>;

  template <class F>
  using MatchReturn = InvokeResult_t<F, T>;

  template <typename OkFunc, typename ErrFunc>
  auto match(OkFunc func_ok, ErrFunc func_err) && -> MatchReturn<OkFunc>;

  template <class F>
  using MapReturn = Result<InvokeResult_t<F, T>, E>;

  template <typename F>
  auto map(F func) && -> MapReturn<F>;

  template <class F>
  using ErrMapReturn = Result<T, InvokeResult_t<F, E>>;

  template <typename F>
  auto map_err(F func) && -> ErrMapReturn<F>;

  template <typename U>
  auto zip(Result<U, E>) && -> Result<std::pair<T, U>, E>;

  template <class F>
  using AndThenReturn = Result<typename InvokeResult_t<F, T>::value_t, E>;

  template <typename F /* T -> Result<U, E> */>
  auto and_then(F func) && -> AndThenReturn<F>;

  template <class F>
  using OrElseReturn = Result<T, typename InvokeResult_t<F, E>::error_t>;

  template <typename F>
  auto or_else(F alt_func) && -> OrElseReturn<F>;
};

template <class T>
auto ok_val(T&& val) -> MakeOkResult<T&&>
{
  return { std::forward<T>(val) };
}

}

template <class T, class E>
bool operator==(const fun::MakeOkResult<T>& a, const fun::Result<T, E>& b) {
  return b == a;
}

template <class T, class E>
bool operator==(const fun::MakeErrResult<E>& a, const fun::Result<T, E>& b) {
  return b == a;
}

template <class U, class T, class E>
bool operator!=(const U& a, const fun::Result<T, E>& b) {
  return b != a;
}

template <class T, class E>
std::ostream& operator<<(std::ostream& os, const fun::Result<T, E>& res);
