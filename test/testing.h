#ifndef TESTING_H
#define TESTING_H

#include <ostream>
#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace test {

class Tester {
private:
  std::string _name;
  std::ostream& _os;
  mutable bool _passing = true;

public:
  explicit Tester(std::string name, std::ostream& os)
    : _name(std::move(name)), _os(os)
  {}
  
  bool passing() const { return _passing; }
  
  template <class T>
  void assert_true(const T& boolish) const {
    if (boolish) {}
    else {
      _passing = false;
      throw std::runtime_error("assert_true failed");
    }
  }
  
  template <class T>
  void expect_true(const T& boolish) const {
    if (boolish) {}
    else { _passing = false; }
  }

  template <class T>
  void expect_eq(const T& a, const T& b) const {
    if (a == b) {}
    else {
      _os << a << " != " << b << std::endl;
      _passing = false;
    }
  }

  template <class T>
  void assert_eq(const T& a, const T& b) const {
    if (a == b) {}
    else {
      _os << a << " != " << b << std::endl;
      _passing = false;
      throw std::runtime_error("assert_eq failed");
    }
  }

  template <class T>
  void expect_neq(const T& a, const T& b) const {
    if (a != b) {}
    else {
      _os << a << " == " << b << std::endl;
      _passing = false;
    }
  }

  template <class T>
  void assert_neq(const T& a, const T& b) const {
    if (a != b) {}
    else {
      _os << a << " == " << b << std::endl;
      _passing = false;
      throw std::runtime_error("assert_neq failed");
    }
  }
  
  template <class F>
  void assert_throw(F f) const {
    try {
      f();
      _os << "expected function to throw, but it did not" << std::endl;
      _passing = false;
    } catch (...) { return; }
    throw std::runtime_error("assert_throw failed");
  }
  
  template <class F>
  void expect_throw(F f) const {
    try {
      f();
      _os << "expected function to throw, but it did not" << std::endl;
      _passing = false;
    } catch (...) {}
  }
};

using UnitTest = std::function<void(const test::Tester&)>;

//------------------------------------------------------------------------------
void run_tests( const std::unordered_map<const char*, UnitTest>& test_map
              , std::ostream& os
              )
{
  size_t n_passed = 0;
  for (const auto& key_val : test_map) {
    auto test_name = key_val.first;
    const auto& test_fn = key_val.second;
    
    std::stringstream test_output;
    const auto tester = test::Tester(std::string(test_name), test_output);
    try {
      test_fn(tester);
      n_passed += tester.passing() ? 1 : 0;
    } catch (const std::runtime_error& err) {
      test_output << "caught exception: " << err.what() << std::endl;
    }
    
    const char* verdict = tester.passing() ? "passed" : "**FAILED**";
    os << "[" << test_name << ": " << verdict << "]" << std::endl;
    const auto details = test_output.str();
    if (!details.empty()) {
      os << details << std::endl;
    }
  }
  
  os << "\n"
     << n_passed << " of " << test_map.size()
     << " tests passed" << std::endl
     ;
}

} // end namespace

#endif
