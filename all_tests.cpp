
// #include "Result.h"
// #include "Option.tmpl.h"
#include "fun/result.h"

#include "testing.h"

#include <memory>
#include <iostream>
#include <vector>

void do_nothing() {}

//------------------------------------------------------------------------------
std::vector<double> example_vector() {
  return std::vector<double>({1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
}

//------------------------------------------------------------------------------
auto example_unique_one() -> std::unique_ptr<int> {
  return std::unique_ptr<int>(new int(1));
}

//==============================================================================
const std::unordered_map<const char*, test::UnitTest> tests = {

//------------------------------------------------------------------------------
{ "option_none_constructor", [](const test::Tester& test) {
  fun::Option<int> op;
  test.assert_true(op.is_none());
}},

//------------------------------------------------------------------------------
{"option_none_constructor", [](const test::Tester& test) {
  fun::Option<int> op;
  test.assert_true(op.is_none());
}},

//------------------------------------------------------------------------------
{"option_some_constructor", [](const test::Tester& test) {
  const fun::Option<int> op(3);
  test.assert_true(op.is_some());
}},

//------------------------------------------------------------------------------
{"option_some_reference_constructor", [](const test::Tester& test) {
  const auto x = 3;
  const fun::Option<const int&> op(x);
  test.assert_true(op.is_some());
}},

//------------------------------------------------------------------------------
{"option_forwarding_constructor", [](const test::Tester& test) {
  const fun::Option<std::vector<int>> op = fun::make_some(3, 0);
  test.assert_true(op.is_some());
}},

//------------------------------------------------------------------------------
{"option_emplace", [](const test::Tester& test) {
  auto op = fun::Option<std::vector<int>>();
  test.assert_true(op.is_none());

  op.emplace(5, 0);
  test.assert_true(op.is_some());
}},

//------------------------------------------------------------------------------
{"option_equality_operator", [](const test::Tester& test) {
  const auto a = fun::some(2.0);
  const auto b = fun::some(2.0);
  const auto c = fun::some(1.0);

  test.assert_true(a == b);
  test.assert_true(b != c);
}},

//------------------------------------------------------------------------------
{"option_as_ref", [](const test::Tester& test) {
  auto x = fun::some(5);
  const auto x_ref = x.as_ref();
  test.assert_true(x_ref.is_some());
  test.assert_true(x_ref.clone().unwrap() == 5);
}},

//------------------------------------------------------------------------------
{"option_as_const_ref", [](const test::Tester& test) {
  const auto x = fun::some(5);
  const auto x_ref = x.as_ref();
  test.assert_true(x_ref);
  test.assert_true(x_ref.clone().unwrap() == 5);
}},

//------------------------------------------------------------------------------
{"option_iterators", [](const test::Tester& test) {
  {
    auto op = fun::Option<double>(1.5);
    size_t n = 0;
    for (const auto& x : op) {
      ++n;
      test.expect_eq(x, 1.5);
    }
    test.expect_eq(n, size_t(1));
  }
  {
    auto op = fun::Option<double>();
    size_t n = 0;
    for (const auto& x : op) {
      ++n;
    }
    test.expect_eq(n, size_t(0));
  }
}},

//------------------------------------------------------------------------------
{"option_implicit_bool_conversion", [](const test::Tester& test) {
  {
    size_t n = 0;
    if (auto op = fun::Option<double>(1.5)) {
      ++n;
      test.expect_eq(op.clone().unwrap(), 1.5);
    }
    test.expect_eq(n, size_t(1));
  }
  {
    size_t n = 0;
    if (auto op = fun::Option<double>()) {
      ++n;
    }
    test.expect_eq(n, size_t(0));
  }
}},

//------------------------------------------------------------------------------
{"option_match", [](const test::Tester& test) {
  using std::vector;

  const auto y = 10.0;
  {
    auto op = fun::some(example_vector());
    const auto xs = std::move(op).match(
      [&](vector<double> vec){
        vec.push_back(y); return vec;
      },
      [](){
        return vector<double>();
      }
    );
    test.assert_eq(xs.size(), example_vector().size() + 1);
  }
  {
    const auto op = fun::some(example_vector());
    const auto x = op.as_ref().match(
      [&](const vector<double>& vec) { return vec.size() + y; },
      [&]() { return -y; }
    );
    test.assert_eq(x, example_vector().size() + y);
  }
}},

//------------------------------------------------------------------------------
{"option_match_void", [](const test::Tester& test) {
  using std::vector;

  fun::some(example_vector()).match(
    [&](vector<double> vec) {
      test.assert_eq(vec.size(), example_vector().size());
    },
    [&]() {
      test.assert_true(false);
    }
  );

  const auto op = fun::some(example_vector());
  op.as_ref().match(
    [&](const vector<double>& vec) {
      test.assert_eq(vec.size(), size_t(6));
    },
    [&]() {
      test.assert_true(false);
    }
  );
}},

//------------------------------------------------------------------------------
{"option_map", [](const test::Tester& test) {
  using std::vector;

  const auto xs = fun::some(example_vector())
    .map(
      [](vector<double> vec) {
        vec.push_back(7.0);
        return vec;
      }
    )
    .unwrap();

  test.assert_eq(xs.size(), example_vector().size() + 1);
}},

//------------------------------------------------------------------------------
{"option_map_void", [](const test::Tester& test) {
  using std::vector;

  auto xs = example_vector();
  const auto unit = fun::some_ref(xs).map(
    [](vector<double>& vec) -> void { vec.push_back(7.0); }
  );

  test.assert_eq(xs.size(), example_vector().size() + 1);
}},

//------------------------------------------------------------------------------
{"option_unvoid", [](const test::Tester& test) {
  const auto _ = fun::unvoid_call(do_nothing);
  // const UnvoidCall<true, decltype(do_nothing)>::Output foo;
}},

//------------------------------------------------------------------------------
{"option_map_or", [](const test::Tester& test) {
  using std::vector;

  const auto xs = fun::some(example_vector()).map_or(vector<double>(),
    [](vector<double> vec) {
      vec.push_back(7.0);
      return vec;
    }
  );

  test.assert_eq(xs.size(), example_vector().size() + 1);
}},

//------------------------------------------------------------------------------
{"option_bind", [](const test::Tester& test) {
  using std::vector;

  auto xs = example_vector();
  auto last = fun::some(&xs).and_then(
    [](vector<double>& vec) {
      vec.push_back(7.0);
      return fun::some(7.0);
    }
  );

  test.assert_eq(xs.size(), example_vector().size() + 1);
  test.assert_eq(last.clone().unwrap(), 7.);
}},

//------------------------------------------------------------------------------
{"option_iteration", [](const test::Tester& test) {
  using std::vector;
  
  auto xs = example_vector();
  auto maybe_xs = fun::some(&xs);
  for (auto& xs : maybe_xs) {
      xs.push_back(7.0);
  }

  test.assert_eq(xs.size(), example_vector().size() + 1);
  
  auto maybe_ys = fun::some(example_vector());
  for (auto& ys : maybe_ys) {
      ys.push_back(7.0);
  }
  const auto p_ys = maybe_ys.as_ptr();
  const auto n_ys = p_ys ? p_ys->size() : 0;

  test.assert_eq(n_ys, example_vector().size() + 1);
}},

//------------------------------------------------------------------------------
{"option_unwrap_or", [](const test::Tester& test) {
  const auto x = fun::some(2).unwrap_or(0);
  test.assert_true(x == 2);
}},

//------------------------------------------------------------------------------
{"option_equality", [](const test::Tester& test) {
  const auto x = fun::some(2);
  const auto y = fun::some(2);
  const auto z = fun::Option<int>();
  test.expect_eq(x, y);
  test.expect_neq(z, x);
}},

//------------------------------------------------------------------------------
{"option_expect", [](const test::Tester& test) {
  const auto x = fun::Option<int>();
  test.assert_throw([&](){ int y = x.clone().expect("error message"); });
}},

//------------------------------------------------------------------------------
{"result_construction", [](const test::Tester& test) {
  const auto x = fun::return_ok<std::string>(3);
  test.assert_eq(x.clone().unwrap(), 3);
  const auto y = fun::return_err<int>(std::string("fail"));
  test.assert_eq(y.clone().unwrap_err().size(), size_t(4));
  auto z = fun::Result<std::vector<double>, std::string>(
    fun::OkTag{}, fun::ForwardArgs{}, 5, 2.0
  );
  test.assert_eq(std::move(z).unwrap().size(), size_t(5));
  const auto xs = example_vector();
  const auto xs_ref = fun::return_ok_ref<std::string>(xs);
  test.assert_eq(xs_ref.clone().unwrap().size(), xs.size());
  const auto xs_err_ref = fun::return_err_ref<std::string>(xs);
  test.assert_eq(xs_err_ref.clone().unwrap_err().size(), xs.size());
}},

//------------------------------------------------------------------------------
{"result_unwrap", [](const test::Tester& test) {
  auto p = fun::return_ok<std::string>(example_unique_one()).unwrap();
  test.assert_true(p.get());
}},

//------------------------------------------------------------------------------
{"result_equality", [](const test::Tester& test) {
  using std::string;
  
  const auto a = fun::return_ok<string>(5.0);
  const auto b = fun::return_ok<string>(5.0);
  const auto c = fun::return_ok<string>(4.0);
  const auto d = fun::return_err<double>(string("not a number"));
  const auto e = fun::return_err<double>(string("not a number"));
  
  test.assert_eq(a, b);
  test.assert_neq(a, c);
  test.assert_neq(a, d);
  test.assert_eq(d, e);
}},

//------------------------------------------------------------------------------
{"result_into_option", [](const test::Tester& test) {
  using std::string;
  using std::unique_ptr;
  
  const auto a = fun::return_ok<string>(example_unique_one()).ok();
  const auto n = a.as_ref().map([](const unique_ptr<int>& p) -> int { return *p; })
                  .unwrap_or(0);
  test.assert_eq(n, 1);
  
  const auto b = fun::return_err<string>(example_unique_one()).err();
  const auto m = b.as_ref().map([](const unique_ptr<int>& p) -> int { return *p; })
                  .unwrap_or(0);
  test.assert_eq(m, 1);
}}

}; // end of test map

//------------------------------------------------------------------------------

namespace fun {

template <class T>
auto pipe(T x) -> T { return x; }

template <class T, class F, class ...Args>
auto pipe(T x, F f, Args&& ...args) { return pipe(f(std::move(x)), std::forward<Args>(args)...); }

} // end namespace fun

class Foo {
public:
  Foo() = delete;
  Foo(int, double) {}
};

class CryBaby {
  int _n = 0;
  
public:
  ~CryBaby() {
    std::cout << "waaaaa destruction!" << std::endl;
  }

  using Self = CryBaby;
  CryBaby(const Self& other) {
    std::cout << "waaaaa copy construction!" << std::endl;
  }
  auto operator=(const Self& other) -> Self& {
    std::cout << "waaaaa copy assign!" << std::endl;
    return *this;
  }
  
  CryBaby(Self&& other) {
    std::cout << "waaaaa move construction!" << std::endl;
  }
  auto operator=(Self&& other) -> Self& {
    std::cout << "waaaaa move assign!" << std::endl;
    return *this;
  }
  
  CryBaby() = default;
  
  void cry() const { std::cout << "waaaaa!" << std::endl; }
};

auto foobar(int n) -> fun::Result<fun::Option<Foo>, int> {
  const double x = 2 * n;
  // return MAKE_OK(MAKE_SOME(2 * n));
  return { fun::OkTag{}, fun::ForwardArgs{}, fun::ForwardArgs{}, n, x };
}

auto good_int(const int n) -> fun::Result<Foo, int>
{
  const auto x = 2. * static_cast<double>(n);
  if (n % 2 == 0) {
    return fun::make_ok(n, x);
  }
  else {
    return fun::return_err(n);
  }
}

auto even_baby(const int n) -> fun::Option<CryBaby> {
  if (n % 2 == 0) { return fun::make_some<CryBaby>(CryBaby()); }
  else            { return {}; }
}

int main(int nargs, char** vargs) {
  test::run_tests(tests, std::cout);
  
  const auto safe_cstr = [](std::string s) -> fun::Option<std::string> {
    if (s.empty()) { return {}; }
    else           { return fun::some(std::move(s)); }
  };
  const auto small_str = [](std::string s) -> fun::Option<std::string> {
    if (s.size() < 3) { return {}; }
    else              { return fun::some(std::move(s)); }
  };
  const auto y = pipe( fun::some(std::string("345"))
                     , fun::bind_op(safe_cstr)
                     , fun::bind_op(small_str)
                     ).unwrap_or(std::string("failure"));
  std::cout << y << std::endl;
  
  const auto x = fun::make_ok(y, 6., "hal:");
  const auto baby = even_baby(nargs * 2.);
  // baby.as_ref().map([](const auto& b) { b.cry(); });
  // const auto baby2 = baby;
  // baby2.as_ref().map([](const auto& b) { b.cry(); });
  
  std::cout << "finished" << std::endl;
}
