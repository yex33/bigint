#include <iostream>
#include <stdexcept>
#include <string_view>

#include "bigint.hpp"

int passed_cnt = 0;
int failed_cnt = 0;

// Constructor tests
void test(bool is_passed, std::string_view test_name) {
  if (is_passed) {
    ++passed_cnt;
  } else {
    ++failed_cnt;
    std::cout << "[FAIL] " << test_name << "\n";
  }
}

void test_constructor() {
  try {
    bigint b1(111111111222);
    test(true, "[bigint] constructor - valid input 1");
  } catch (...) {
    test(false, "[bigint] constructor - valid input 1");
  }

  try {
    bigint b2("1111111112222222223");
    test(true, "[bigint] constructor - valid input 2");
  } catch (...) {
    test(false, "[bigint] constructor - valid input 2");
  }
}

void test_paces() {
  try {
    bigint b2("   12");
    test(false, "[bigint] leading spaces should throw");
  } catch (const std::invalid_argument &) {
    test(true, "[bigint] leading spaces should throw");
  }

  try {
    bigint b4("1   2");
    test(false, "[bigint] internal spaces should throw");
  } catch (const std::invalid_argument &) {
    test(true, "[bigint] internal spaces should throw");
  }
}

void test_invalid_sign() {
  try {
    bigint b1("--12");
    test(false, "[bigint] double '-' should throw");
  } catch (const std::invalid_argument &) {
    test(true, "[bigint] double '-' should throw");
  }

  try {
    bigint b2("-1-2");
    test(false, "[bigint] '-' in the middle should throw");
  } catch (const std::invalid_argument &) {
    test(true, "[bigint] '-' in the middle should throw");
  }

  try {
    bigint b3("-12-");
    test(false, "[bigint] trailing '-' should throw");
  } catch (const std::invalid_argument &) {
    test(true, "[bigint] trailing '-' should throw");
  }
}

void test_invalid_character() {
  try {
    bigint b1("12?");
    test(false, "[bigint] invalid '?' should throw");
  } catch (const std::invalid_argument &) {
    test(true, "[bigint] invalid '?' should throw");
  }
}

// Addition tests
void test_additions() {
  // Addition of zero
  test(bigint(0) + bigint(0) == bigint(0), "[bigint] 0 + 0");
  test(bigint(12345) + bigint(0) == bigint(12345), "[bigint] 12345 + 0");
  test(bigint(0) + bigint(12345) == bigint(12345), "[bigint] 0 + 12345");

  // Positive number addition
  test(bigint(12345) + bigint(67890) == bigint(80235),
       "[bigint] 12345 + 67890");
  test(bigint(999999) + bigint(1) == bigint(1000000), "[bigint] 999999 + 1");

  // Negative number addition
  test(bigint(-12345) + bigint(-67890) == bigint(-80235),
       "[bigint] -12345 + -67890");
  test(bigint(-999999) + bigint(-1) == bigint(-1000000),
       "[bigint] -999999 + -1");

  // Mixed sign addition
  test(bigint(12345) + bigint(-67890) == bigint(-55545),
       "[bigint] 12345 + -67890");
  test(bigint(-12345) + bigint(67890) == bigint(55545),
       "[bigint] -12345 + 67890");

  // Addition resulting in zero
  test(bigint(12345) + bigint(-12345) == bigint(0), "[bigint] 12345 + -12345");

  // Large number addition
  test(bigint("1234567891011121314151617181920") + bigint("987654321098765432101234567890") ==
           bigint("2222222212109886746252851749810"), "[bigint] large number addition 1");
  test(bigint("999999999999999999999999999999") + bigint("1") ==
           bigint("1000000000000000000000000000000"), "[bigint] large number addition 2");

  // Addition chaining
  test(bigint(12345) + bigint(67890) + bigint(11111) == bigint(91346),
       "[bigint] 12345 + 67890 + 11111");
}

// Multiplication tests
void test_multiplications() {
  // Multiplication with zero
  test(bigint(0) * bigint(0) == bigint(0), "[bigint] 0 * 0");
  test(bigint(12345) * bigint(0) == bigint(0), "[bigint] 12345 * 0");
  test(bigint(0) * bigint(12345) == bigint(0), "[bigint] 0 * 12345");

  // Multiplication with one
  test(bigint(12345) * bigint(1) == bigint(12345), "[bigint] 12345 * 1");
  test(bigint(1) * bigint(12345) == bigint(12345), "[bigint] 1 * 12345");

  // Positive number multiplication
  test(bigint(123) * bigint(456) == bigint(56088), "[bigint] 123 * 456");
  test(bigint(999) * bigint(999) == bigint(998001), "[bigint] 999 * 999");

  // Negative number multiplication
  test(bigint(-123) * bigint(456) == bigint(-56088), "[bigint] -123 * 456");
  test(bigint(-123) * bigint(-456) == bigint(56088), "[bigint] -123 * -456");
}

// Subtraction tests
void test_subtractions() {
  // Subtraction resulting in zero
  test(bigint(0) - bigint(0) == bigint(0), "[bigint] 0 - 0");
  test(bigint(12345) - bigint(12345) == bigint(0), "[bigint] 12345 - 12345");
  test(bigint(-12345) - bigint(-12345) == bigint(0), "[bigint] -12345 - -12345");

  // Subtracting zero
  test(bigint(12345) - bigint(0) == bigint(12345), "[bigint] 12345 - 0");

  // Positive number subtraction
  test(bigint(12345) - bigint(54321) == bigint(-41976), "[bigint] 12345 - 54321");
}

// Comparison tests
void test_equality() {
    test(bigint() == bigint(), "[bigint] equality default constructor");
    test(bigint(0) == bigint(0), "[bigint] equality zero");
    test(bigint(12345) == bigint(12345), "[bigint] equality positive numbers");
    test(bigint(-12345) == bigint(-12345), "[bigint] equality negative numbers");
    test(bigint("123456789101112") == bigint(123456789101112), "[bigint] equality string vs number");
    test(bigint("0") == bigint("-0"), "[bigint] equality string zero negative zero");
    test(bigint("1234567891011121314151617181920") ==
             bigint("1234567891011121314151617181920"),
         "[bigint] equality large number");
}

void test_inequality() {
  test(bigint(12345) != bigint(12346), "[bigint] inequality positive numbers");
  test(bigint(12345) != bigint(-12345),
       "[bigint] inequality positive vs negative");
  test(bigint("12345") != bigint(-12345),
       "[bigint] inequality string vs negative number");
  test(bigint("1234567891011121314151617181920") !=
           bigint("1234567891011121314151617181921"),
       "[bigint] inequality large numbers");
}

void test_less_than() {
    test(bigint(12345) < bigint(12346), "[bigint] less than simple case");
    test(bigint(-12345) < bigint(12345), "[bigint] less than negative vs positive");
    test(bigint("123456789101112") < bigint("123456789101113"),
         "[bigint] less than large numbers");
}

void test_greater_than() {
    test(bigint(12346) > bigint(12345), "[bigint] greater than simple case");
    test(bigint(12345) > bigint(-12345), "[bigint] greater than positive vs negative");
    test(bigint("123456789101113") > bigint("123456789101112"),
         "[bigint] greater than large numbers");
}

int main() {
  test_constructor();
  test_paces();
  test_invalid_sign();
  test_invalid_character();

  test_additions();
  test_multiplications();
  test_subtractions();

  test_equality();
  test_inequality();
  test_less_than();
  test_greater_than();

  std::cout << "Tests Passed: " << passed_cnt << "\n";
  std::cout << "Tests Failed: " << failed_cnt << "\n";

  return 0;
}
