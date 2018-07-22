#ifndef FUN_PIPE_H
#define FUN_PIPE_H

#include <utility>

namespace fun {

template <class T>
auto pipe(T&& x) -> T { return std::forward<T>(x); }

template <class T, class F, class ...Args>
auto pipe(T&& x, F&& f, Args&& ...args) {
  return pipe(f(std::forward<T>(x)), std::forward<Args>(args)...);
}

// template <class F>
// auto compose(F&& f_ref) -> F { return std::forward<F>(f_ref); }
//
// template <class F, class G, class ...Args>
// auto compose(F&& f_ref, G&& g_ref, Args&& ...args) {
//   return compose(
//     [f = std::forward<F>(f_ref), g = std::forward<G>(g_ref)]
//     (auto&& x) { return g(f(std::forward<decltype(x)>(x))); },
//     std::forward<Args>(args)...
//   );
// }

} // end namespace fun

#endif
