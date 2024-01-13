#pragma once

//!
//! @author Alex Pronschinske
//! @copyright MIT LIcense
//!

#include <fun/result/result.declare.h>
#include <fun/option/option.impl.h>

namespace fun {

//==============================================================================
// Result-releated function definitions
//------------------------------------------------------------------------------
template <class T>
auto ok(T val) -> MakeOkResult<T> {
  return { std::forward<T>(val) };
}

//------------------------------------------------------------------------------
template <class E, class T>
auto ok(T val) -> Result<T, E> {
  return { OkTag{}, ForwardArgs{}, std::forward<T>(val) };
}

//------------------------------------------------------------------------------
template <class T>
auto ok_cref(const T& val) -> MakeOkResult<const T&> {
  return { val };
}

//------------------------------------------------------------------------------
template <class T>
auto ok_ref(T& val) -> MakeOkResult<T&> {
  return { val };
}

//------------------------------------------------------------------------------
template <class E, class T>
auto ok_ref(T& val) -> Result<T&, E> {
  return { OkTag{}, ForwardArgs{}, val };
}

//------------------------------------------------------------------------------
template <class E>
auto err(E val) -> MakeErrResult<E> {
  return { std::forward<E>(val) };
}

//------------------------------------------------------------------------------
template <class T, class E>
auto err(E val) -> Result<T, E> {
  return { ErrTag{}, ForwardArgs{}, std::forward<E>(val) };
}

//------------------------------------------------------------------------------
template <class E>
auto err_cref(const E& val) -> MakeErrResult<const E&> {
  return { val };
}

//------------------------------------------------------------------------------
template <class E>
auto err_ref(E& val) -> MakeErrResult<E&> {
  return { val };
}

//------------------------------------------------------------------------------
template <class T, class E>
auto err_ref(E& val) -> Result<T, E&> {
  return { ErrTag{}, ForwardArgs{}, val };
}

//==============================================================================
// Option definitions
//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::dump_ok() -> T {
  assert(_variant == Ok);
  return std::move(_ok).unwrap();
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::dump_err() -> E {
  assert(_variant == Err);
  return std::move(_err).unwrap();
}

//------------------------------------------------------------------------------
template <class T, class E>
Result<T, E>::~Result() {
  if (_variant == Ok) { _ok.~Sized<T>(); }
  else                { _err.~Sized<E>(); }
}

//------------------------------------------------------------------------------
template <class T, class E>
Result<T, E>::Result(self_t&& other)
  : _variant(other._variant)
{
  if (_variant == Ok) { new (&_ok)  Sized<T>{ other.dump_ok() }; }
  else                { new (&_err) Sized<E>{ other.dump_err() }; }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::operator=(self_t&& other) -> self_t& {
  if (this != &other) {
    (*this).~Result<T, E>();
    new (this) self_t(std::move(other));
  }
  return *this;
}

//------------------------------------------------------------------------------
template <class T, class E>
Result<T, E>::Result(const self_t& other)
  : _variant(other._variant)
{
  if (_variant == Ok) { new (&_ok) Sized<T>(other._ok.val()); }
  else                { new (&_err) Sized<E>(other._err.val()); }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::operator=(const self_t& other) -> self_t& {
  if (this != &other) {
    (*this).~Result<T, E>();
    new (this) self_t(other);
  }
  return *this;
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::operator=(const MakeOkResult<T>& other) -> self_t& {
  (*this).~Result<T, E>();
  new (this) Result<T, E>(other);
  return *this;
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::operator=(const MakeErrResult<E>& other) -> self_t& {
  (*this).~Result<T, E>();
  new (this) Result<T, E>(other);
  return *this;
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::clone() const -> self_t { return self_t(*this); }

//------------------------------------------------------------------------------
template <class T, class E>
template <typename ...Args>
Result<T, E>::Result(OkTag, ForwardArgs, Args&& ...args)
  : _variant(Ok), _ok(Sized<T>(std::forward<Args>(args)...))
{}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename ...Args>
Result<T, E>::Result(ErrTag, ForwardArgs, Args&& ...args)
  : _variant(Err), _err(Sized<E>(std::forward<Args>(args)...))
{}

//------------------------------------------------------------------------------
template <class T, class E>
Result<T, E>::Result(MakeOkResult<T> ok)
  : Result(OkTag{}, ForwardArgs{}, std::forward<T>(ok.val))
{}

//------------------------------------------------------------------------------
template <class T, class E>
Result<T, E>::Result(MakeErrResult<E> err)
  : Result(ErrTag{}, ForwardArgs{}, std::forward<E>(err.val))
{}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::is_ok() const -> bool { return _variant == Ok; }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::is_err() const -> bool { return !is_ok(); }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_ptr() -> value_t* { return is_ok() ? &_ok.val() : nullptr; }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_ptr() const -> const value_t* { return is_ok() ? &_ok.val() : nullptr; }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_err_ptr() -> error_t* { return is_err() ? &_err.val() : nullptr; }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_err_ptr() const -> const error_t* { return is_err() ? &_err.val() : nullptr; }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::operator==(const self_t& other) const -> bool {
  if (is_ok()) {
    return other.is_ok() ? (_ok.val() == other._ok.val()) : false;
  } else {
    return other.is_err() ? (_err.val() == other._err.val()) : false;
  }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::operator!=(const self_t& other) const -> bool { return !(*this == other); }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::unwrap() && -> T { return dump_ok(); }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::unwrap_or(T alt) && -> T {
  if (is_ok()) { return dump_ok(); }
  else         { return std::forward<T>(alt); }
}

//------------------------------------------------------------------------------
template <class T, class E>
template <class F>
auto Result<T, E>::unwrap_or_else(F&& alt_func) && -> T {
  return std::move(*this).match(
    [](T&& x) -> T { return std::forward<T>(x); },
    [&](E&& err) -> T { return unvoid_call(std::forward<F>(alt_func), std::forward<E>(err)); }
  );
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::unwrap_err() && -> E { return dump_err(); }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_ref() -> Result<value_t&, error_t&> {
  if (is_ok()) { return { OkTag{}, ForwardArgs{}, _ok.val() }; }
  else         { return { ErrTag{}, ForwardArgs{}, _err.val() }; }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_ref() const -> Result<const value_t&, const error_t&> {
  if (is_ok()) { return { OkTag{}, ForwardArgs{}, _ok.val() }; }
  else         { return { ErrTag{}, ForwardArgs{}, _err.val() }; }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_cref() const -> Result<const value_t&, const error_t&> {
  return as_ref();
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::ok() && -> Option<T> {
  if (is_ok()) { return Option<T>{ ForwardArgs{}, dump_ok() }; }
  else         { return {}; }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::err() && -> Option<E> {
  if (is_err()) { return Option<E>{ ForwardArgs{}, dump_err() }; }
  else          { return {}; }
}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename OkFunc, typename ErrFunc>
auto Result<T, E>::match(OkFunc&& func_ok, ErrFunc&& func_err) && -> MatchReturn<OkFunc> {
  static_assert(
    std::is_same_v<std::invoke_result_t<OkFunc, T>, std::invoke_result_t<ErrFunc, E>>
    , "Ok-handling and Err-handling functions passed to match do not "
      "have the same return type"
    );
  if (is_ok()) { return unvoid_call(std::forward<OkFunc>(func_ok), dump_ok()); }
  else         { return unvoid_call(std::forward<ErrFunc>(func_err), dump_err()); }
}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename F>
auto Result<T, E>::map(F&& func) && -> MapReturn<F> {
  if (is_ok()) { return { OkTag{}, ForwardArgs{}, unvoid_call(std::forward<F>(func), dump_ok()) }; }
  else         { return { ErrTag{}, ForwardArgs{}, dump_err() }; }
}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename F>
auto Result<T, E>::map_err(F&& func) && -> ErrMapReturn<F> {
  if (is_err()) { return { ErrTag{}, ForwardArgs{}, unvoid_call(std::forward<F>(func), dump_err()) }; }
  else          { return { OkTag{}, ForwardArgs{}, dump_ok() }; }
}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename U>
auto Result<T, E>::
zip(Result<U, E> other) && -> Result<std::pair<T, U>, E> {
  if (this->is_ok() && other.is_ok()) {
    return fun::make_ok(std::move(*this).unwrap(), std::move(other).unwrap());
  } else if (this->is_ok()) {
    return fun::make_err(std::move(other).unwrap_err());
  } else {
    return fun::make_err(std::move(*this).unwrap_err());
  }
}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename F /* T -> Result<U, E> */>
auto Result<T, E>::and_then(F&& func) && -> AndThenReturn<F> {
    if (is_ok()) { return unvoid_call(std::forward<F>(func), dump_ok()); }
    else         { return { ErrTag{}, ForwardArgs{}, dump_err() }; }
}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename F>
auto Result<T, E>::or_else(F&& alt_func) && -> OrElseReturn<F> {
  if (is_ok()) { return { OkTag{}, ForwardArgs{}, dump_ok() }; }
    else       { return unvoid_call(std::forward<F>(alt_func), dump_err()); }
}

}

//------------------------------------------------------------------------------
template <class T, class E>
std::ostream& operator<<(std::ostream& os, const fun::Result<T, E>& res) {
  return res.as_ref().match(
    [&](const T& x) -> std::ostream& {
      os << "Ok(" << x << ")";
      return os;
    },
    [&](const E& y) -> std::ostream& {
      os << "Err(" << y << ")";
      return os;
    }
  );
}
