#ifndef FUN_RESULT_TEMPLATE_DECLARATIONS_H
#define FUN_RESULT_TEMPLATE_DECLARATIONS_H

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
auto return_ok(T val) -> MakeOkResult<T>;

template <class E, class T>
auto return_ok(T val) -> Result<T, E>;

template <class T>
auto return_ok_cref(const T& val) -> MakeOkResult<const T&>;
template <class T>
auto return_ok_ref(T& val) -> MakeOkResult<T&>;

template <class E, class T>
auto return_ok_ref(T& val) -> Result<T&, E>;

template <class E>
auto return_err(E val) -> MakeErrResult<E>;

template <class T, class E>
auto return_err(E val) -> Result<T, E>;

template <class E>
auto return_err_cref(const E& val) -> MakeErrResult<const E&>;
template <class E>
auto return_err_ref(E& val) -> MakeErrResult<E&>;

template <class T, class E>
auto return_err_ref(E& val) -> Result<T, E&>;

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
  
  using ok_value_t = std::remove_reference_t<T>;
  using err_value_t = std::remove_reference_t<E>;

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

  bool is_ok() const;
  bool is_err() const;

  ok_value_t* as_ptr();
  err_value_t* as_err_ptr();

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

  T unwrap();
  E unwrap_err();
  
  auto as_ref() -> Result<ok_value_t&, err_value_t&>;
  
  auto as_ref() const -> Result<const ok_value_t&, const err_value_t&>;
  
  auto as_cref() const -> Result<const ok_value_t&, const err_value_t&>;

  auto ok() && -> Option<T>;
  auto err() && -> Option<E>;
  
  template <class F>
  using MatchReturn = ResultOf_t<F(T)>;

  template <typename OkFunc, typename ErrFunc>
  auto match(OkFunc func_ok, ErrFunc func_err) && -> MatchReturn<OkFunc>;

  template <class F>
  using MapReturn = Result<ResultOf_t<F(T)>, E>;

  template <typename F>
  auto map(F func) && -> MapReturn<F>;

  template <class F>
  using ErrMapReturn = Result<T, ResultOf_t<F(T)>>;

  template <typename F>
  auto map_err(F func) && -> ErrMapReturn<F>;

  template <class F>
  using AndThenReturn = Result<typename ResultOf_t<F(T)>::OkT, E>;

  template <typename F /* T -> Result<U, E> */>
  auto and_then(F func) && -> AndThenReturn<F>;

  template <class F>
  using OrElseReturn = Result<T, typename ResultOf_t<F(T)>::ErrT>;

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

template <class T, class E>
std::ostream& operator<<(std::ostream& os, const fun::Result<T, E>& res);

#endif
