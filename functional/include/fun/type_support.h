#ifndef TYPE_SUPPORT_H
#define TYPE_SUPPORT_H

#include <type_traits>
#include <functional>

namespace fun {

//------------------------------------------------------------------------------
struct Unit {
  using Self = Unit;

  constexpr bool operator==(const Self&) const { return true; }
  constexpr bool operator!=(const Self&) const { return false; }
};

//------------------------------------------------------------------------------
struct ForwardArgs {};

//------------------------------------------------------------------------------
template <class T> struct Sized;

template <class T>
struct Sized<T&> {
  std::reference_wrapper<T> _val;

  Sized(T& ref) : _val(ref) {}
  Sized(const std::reference_wrapper<T> ref) : _val(ref) {}

  Sized(const Sized<T&>& other) : _val(other._val) {}

  auto val() -> T& { return _val; }
  auto val() const -> const T& { return _val; }

  auto unwrap() && -> T& { return _val; }
};

template <class T>
struct Sized {
  T _val;

  template <class ...Args>
  Sized(Args&&... args) : _val(std::forward<Args>(args)...)
  {}

  auto val() -> T& { return _val; }
  auto val() const -> const T& { return _val; }

  auto unwrap() && -> T { return std::move(_val); }
};

//------------------------------------------------------------------------------
template <class T> struct Unvoid;
template <> struct Unvoid<void> { using Output = Unit; };
template <class T> struct Unvoid { using Output = T; };

//------------------------------------------------------------------------------
template <class T>
struct ResultOf {
  using type = typename Unvoid<typename std::result_of<T>::type>::Output;
};

template <class T>
using ResultOf_t = typename ResultOf<T>::type;

//------------------------------------------------------------------------------
template <bool unvoid, class F, class ...Args> struct UnvoidedFunc;

template <class F, class ...Args>
struct UnvoidedFunc<true, F, Args...> {
  using Output = ResultOf_t<F(Args...)>;

  auto operator()(F f, Args&& ...args) const -> Output {
    f(std::forward<Args>(args)...);
    return {};
  }
};

template <class F, class ...Args>
struct UnvoidedFunc<false, F, Args...> {
  using Output = ResultOf_t<F(Args...)>;

  auto operator()(F f, Args&& ...args) const -> Output {
    return f(std::forward<Args>(args)...);
  }
};

template <class F, class ...Args>
struct UnvoidFunc {
  using Routed = UnvoidedFunc<std::is_void<typename std::result_of<F(Args...)>::type>::value, F, Args...>;
};

template <class F, class ...Args>
auto unvoid_call( F f, Args&& ...args
                ) -> typename UnvoidFunc<F, Args...>::Routed::Output
{
  const typename UnvoidFunc<F, Args...>::Routed uv;
  return uv(f, std::forward<Args>(args)...);
}

//------------------------------------------------------------------------------
template <class T, class... Args>
auto construct_at(T* location, Args&&... args) -> T& {
  ::new (static_cast<void*>(location)) T(std::forward<Args>(args)...);
  return *location;
}

}

#endif
