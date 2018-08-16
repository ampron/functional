#ifndef FUN_PIPE_H
#define FUN_PIPE_H

#include <utility>

namespace fun {

//------------------------------------------------------------------------------
template <class T>
auto pipe(T&& x) -> T { return std::forward<T>(x); }

template <class T, class F, class ...Args>
auto pipe(T&& x, F&& f, Args&& ...args) {
  return pipe(f(std::forward<T>(x)), std::forward<Args>(args)...);
}

//------------------------------------------------------------------------------
template <class F>
auto lift(F&& f) {
  return
    [f = std::forward<F>(f)]
    (auto functor) {
      return std::move(functor).map(f);
    };
}

//------------------------------------------------------------------------------
template <class F>
auto bind(F&& f) {
  return
    [f = std::forward<F>(f)]
    (auto monad) {
      return std::move(monad).and_then(f);
    };
}

} // end namespace fun

#endif
