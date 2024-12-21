#pragma once

#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <ostream>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifdef DOCTEST
#include "doctest.h"
#endif

class bigint {
  using WORD = std::int32_t;

protected:
  // sign == true if n is negative, sign == false if n is positive
  bool sign;
  std::vector<WORD> val;

public:
  bigint() noexcept : sign(false), val{0} {};
  explicit bigint(int64_t n) noexcept;
  /**
   * a constructor that takes a string and a base, and converts the string to an
   * arbitrary-precision integer in that base.
   * @param sv the string to convert
   * @param base the number base
   */
  explicit bigint(std::string_view sv, int base = 10) noexcept(false);

  [[nodiscard]]
  bigint operator+(WORD b) const noexcept;
  const bigint &operator+=(WORD b) noexcept;
  [[nodiscard]]
  bigint operator*(WORD b) const noexcept;
  const bigint &operator*=(WORD b) noexcept;

  [[nodiscard]]
  bigint operator+(const bigint &b) const noexcept;
  const bigint &operator+=(const bigint &b) noexcept;
  [[nodiscard]]
  bigint operator-(const bigint &b) const noexcept;
  const bigint &operator-=(const bigint &b) noexcept;
  [[nodiscard]]
  bigint operator*(const bigint &b) const noexcept;
  const bigint &operator*=(const bigint &b) noexcept;

  [[nodiscard]]
  bigint operator-() const noexcept;

  [[nodiscard]]
  bool operator==(const bigint &b) const noexcept;
  [[nodiscard]]
  bool operator!=(const bigint &b) const noexcept;
  [[nodiscard]]
  bool operator<(const bigint &b) const noexcept;
  [[nodiscard]]
  bool operator>(const bigint &b) const noexcept;
  [[nodiscard]]
  bool operator<=(const bigint &b) const noexcept;
  [[nodiscard]]
  bool operator>=(const bigint &b) const noexcept;

  friend std::ostream &operator<<(std::ostream &os, const bigint &a) noexcept;

private:
  static constexpr WORD WORD_MAX = 9;
  static constexpr WORD BASE = 10;

  void val_plus(const bigint &b, std::size_t offset = 0) noexcept;
  void val_monus(const bigint &b) noexcept;
  void val_mult(const bigint &b) noexcept;

  [[nodiscard]]
  bool val_less(const bigint &b) const noexcept;
  [[nodiscard]]
  bool val_more(const bigint &b) const noexcept;
  [[nodiscard]]
  bool is_zero() const noexcept;
};

inline bigint::bigint(int64_t n) noexcept : sign(false) {
  if (!n) {
    val.push_back(0);
  }
  if (n < 0) {
    sign = true;
  }
  n = std::abs(n);
  WORD chunk = static_cast<WORD>(n % BASE);
  while (n > 0) {
    val.push_back(chunk);
    n /= BASE;
    chunk = static_cast<WORD>(n % BASE);
  }
}

inline bigint::bigint(std::string_view sv, int base) noexcept(false)
    : bigint() {
  bool alnum_flg = false;
  for (char ch : sv) {
    if (std::isalnum(ch)) {
      alnum_flg = true;
      if (std::toupper(ch) - '0' >= base) {
        std::stringstream ss;
        ss << "invalid character " << std::quoted(std::string{ch})
           << " used for integer of base " << base;
        throw std::invalid_argument(ss.str());
      }
    } else if (ch == '-' && !alnum_flg && !sign) {
      sign = true;
    } else {
      std::stringstream ss;
      ss << "invalid character " << std::quoted(std::string{ch}) << " found";
      throw std::invalid_argument(ss.str());
    }
  }
  if (sign) {
    sv = sv.substr(1);
  }
  if (sv.size() == 1 && sv[0] == '0') { // 0 case, make +ve 0
    sign = false;
    return;
  }

  std::size_t wnd_size = 1;

  std::size_t offset = sv.size() % wnd_size;
  WORD wnd;
  std::string_view sub;
  if (offset) {
    sub = sv.substr(0, offset);
    std::from_chars(sub.data(), sub.data() + sub.size(), wnd, base);
    *this += wnd;
  }
  for (std::size_t pos = offset; pos < sv.size(); pos += wnd_size) {
    sub = sv.substr(pos, wnd_size);
    std::from_chars(sub.data(), sub.data() + sub.size(), wnd, base);
    *this *= static_cast<WORD>(std::pow(base, wnd_size));
    *this += wnd;
  }
}

#ifdef DOCTEST_LIBRARY_INCLUDED
TEST_CASE("[bigint] constructs with base 10 by default") {
  CHECK_NOTHROW(bigint _("111111111222222222"));
  CHECK_NOTHROW(bigint _("1111111112222222223"));
}

TEST_CASE("[bigint] spaces should throw invalid_argument") {
  CHECK_THROWS_AS(bigint _(" 12"), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("   12"), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("12   "), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("1   2"), std::invalid_argument);
  CHECK_THROWS_AS(bigint _(" 1  2"), std::invalid_argument);
}

TEST_CASE("[bigint] invalid \"-\" sign should throw invalid_argument") {
  CHECK_THROWS_AS(bigint _("--12"), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("-1-2"), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("-12-"), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("12-"), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("12--"), std::invalid_argument);
}

TEST_CASE("[bigint] invalid character should throw invalid_argument") {
  CHECK_THROWS_AS(bigint _("12?"), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("1?2"), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("?12"), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("*)?"), std::invalid_argument);
}

TEST_CASE("[bigint] out-of-range alnum w.r.t. given base should throw "
          "invalid_argument") {
  CHECK_THROWS_AS(bigint _("123", 3), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("12A", 10), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("12a", 10), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("1G2", 16), std::invalid_argument);
  CHECK_THROWS_AS(bigint _("1g2", 16), std::invalid_argument);
}
#endif

// addition operator

inline bigint bigint::operator+(const WORD b) const noexcept {
  bigint res;
  res += *this;
  res += b;
  return res;
}

inline const bigint &bigint::operator+=(const WORD b) noexcept {
  WORD carry = b;
  for (std::size_t i = 0; i < val.size(); i++) {
    const WORD ai = val[i];
    val[i] = (ai + carry) % BASE;
    carry = (ai + carry) / 10;
    if (!carry)
      break;
  }
  if (carry > 0) {
    val.push_back(carry);
  }
  return *this;
}

inline bigint bigint::operator+(const bigint &b) const noexcept {
  bigint res;
  res += *this;
  res += b;
  return res;
}

inline const bigint &bigint::operator+=(const bigint &b) noexcept {
  if (is_zero()) {
    sign = b.sign;
    val_plus(b);
  } else if (sign == b.sign) {
    val_plus(b);
  } else {
    if (val_less(b)) {
      const bigint a = *this;
      *this = b;
      val_monus(a);
    } else {
      val_monus(b);
    }
    if (is_zero()) {
      sign = false;
    }
  }
  return *this;
}

#ifdef DOCTEST_LIBRARY_INCLUDED
TEST_CASE("operator+ basic functionality") {
  // Addition of zero
  CHECK_EQ(bigint(0) + bigint(0), bigint(0));
  CHECK_EQ(bigint(12345) + bigint(0), bigint(12345));
  CHECK_EQ(bigint(0) + bigint(12345), bigint(12345));

  // Positive number addition
  CHECK_EQ(bigint(12345) + bigint(67890), bigint(80235));
  CHECK_EQ(bigint(999999) + bigint(1), bigint(1000000));

  // Negative number addition
  CHECK_EQ(bigint(-12345) + bigint(-67890), bigint(-80235));
  CHECK_EQ(bigint(-999999) + bigint(-1), bigint(-1000000));

  // Mixed sign addition
  CHECK_EQ(bigint(12345) + bigint(-67890), bigint(-55545));
  CHECK_EQ(bigint(-12345) + bigint(67890), bigint(55545));
  CHECK_EQ(bigint(67890) + bigint(-12345), bigint(55545));

  // Large number addition
  CHECK_EQ(bigint("1234567891011121314151617181920") +
               bigint("987654321098765432101234567890"),
           bigint("2222222212109886746252851749810"));
  CHECK_EQ(bigint("999999999999999999999999999999") + bigint("1"),
           bigint("1000000000000000000000000000000"));
}

TEST_CASE("operator+ edge cases") {
  // Addition resulting in zero
  CHECK_EQ(bigint(12345) + bigint(-12345), bigint(0));
  CHECK_EQ(bigint(-12345) + bigint(12345), bigint(0));

  // Addition with large negative and small positive
  CHECK_EQ(bigint("-1000000000000000000000000") + bigint(1),
           bigint("-999999999999999999999999"));

  // Addition with large positive and small negative
  CHECK_EQ(bigint("1000000000000000000000000") + bigint(-1),
           bigint("999999999999999999999999"));
}

TEST_CASE("operator+ chaining") {
  // Chained addition
  CHECK_EQ(bigint(12345) + bigint(67890) + bigint(11111), bigint(91346));
  CHECK_EQ(bigint(12345) + bigint(-12345) + bigint(67890), bigint(67890));
  CHECK_EQ(bigint(12345) + bigint(0) + bigint(-12345), bigint(0));
}

TEST_CASE("operator+ extreme cases") {
  // Very large numbers
  CHECK_EQ(bigint("123456789123456789123456789") +
               bigint("987654321987654321987654321"),
           bigint("1111111111111111111111111110"));
  CHECK_EQ(bigint("-123456789123456789123456789") +
               bigint("987654321987654321987654321"),
           bigint("864197532864197532864197532"));

  // Numbers close to `BASE` boundaries
  bigint base_minus_one("999999999999999999");
  CHECK_EQ(base_minus_one + bigint(1), bigint("1000000000000000000"));
  CHECK_EQ(base_minus_one + bigint(2), bigint("1000000000000000001"));
}
#endif

// multiplication operator

inline bigint bigint::operator*(const WORD b) const noexcept {
  bigint res;
  res += *this;
  res *= b;
  return res;
}

inline const bigint &bigint::operator*=(const WORD b) noexcept {
  if (is_zero())
    return *this;
  if (!b) {
    val.clear();
    val.push_back(0);
    sign = false;
    return *this;
  }
  WORD carry = 0;
  for (WORD &ai : val) {
    WORD prod = ai * b;
    ai = (prod + carry) % BASE;
    carry = (prod + carry) / BASE;
  }
  if (carry > 0) {
    val.push_back(carry);
  }

  return *this;
}

inline bigint bigint::operator*(const bigint &b) const noexcept {
  bigint res;
  res += *this;
  res *= b;
  return res;
}

inline const bigint &bigint::operator*=(const bigint &b) noexcept {
  sign = (sign != b.sign);
  val_mult(b);
  if (is_zero()) {
    sign = false;
  }
  return *this;
}

#ifdef DOCTEST_LIBRARY_INCLUDED
TEST_CASE("operator* basic functionality") {
  // Multiplication with zero
  CHECK_EQ(bigint(0) * bigint(0), bigint(0));
  CHECK_EQ(bigint(12345) * bigint(0), bigint(0));
  CHECK_EQ(bigint(0) * bigint(12345), bigint(0));

  // Multiplication with one
  CHECK_EQ(bigint(12345) * bigint(1), bigint(12345));
  CHECK_EQ(bigint(1) * bigint(12345), bigint(12345));

  // Positive number multiplication
  CHECK_EQ(bigint(123) * bigint(456), bigint(56088));
  CHECK_EQ(bigint(999) * bigint(999), bigint(998001));

  // Negative number multiplication
  CHECK_EQ(bigint(-123) * bigint(456), bigint(-56088));
  CHECK_EQ(bigint(123) * bigint(-456), bigint(-56088));
  CHECK_EQ(bigint(-123) * bigint(-456), bigint(56088));
}

TEST_CASE("operator* edge cases") {
  // Multiplication resulting in zero
  CHECK_EQ(bigint(12345) * bigint(0), bigint(0));
  CHECK_EQ(bigint(0) * bigint(12345), bigint(0));
  CHECK_EQ(bigint(-12345) * bigint(0), bigint(0));
  CHECK_EQ(bigint(0) * bigint(-12345), bigint(0));

  // Multiplication resulting in a negative number
  CHECK_EQ(bigint(12345) * bigint(-1), bigint(-12345));
  CHECK_EQ(bigint(-12345) * bigint(1), bigint(-12345));
  CHECK_EQ(bigint(-12345) * bigint(-1), bigint(12345));

  // Multiplication with large numbers
  CHECK_EQ(bigint("1000000000000000000") * bigint("1000000000000000000"),
           bigint("1000000000000000000000000000000000000"));
}

TEST_CASE("operator* chaining") {
  // Chained multiplication
  CHECK_EQ(bigint(2) * bigint(3) * bigint(4), bigint(24));
  CHECK_EQ(bigint(10) * bigint(-5) * bigint(2), bigint(-100));
  CHECK_EQ(bigint(-2) * bigint(-3) * bigint(-4), bigint(-24));
}

TEST_CASE("operator* extreme cases") {
  // Very large numbers
  CHECK_EQ(bigint("123456789123456789123456789123456789") *
               bigint("987654321987654321987654321987654321"),
           bigint("121932631356500531591068431825636331816338969581771069347203"
                  "169112635269"));

  // Numbers close to `BASE` boundaries
  bigint base_minus_one("999999999999999999");
  CHECK_EQ(base_minus_one * bigint(2), bigint("1999999999999999998"));
  CHECK_EQ(base_minus_one * bigint(10), bigint("9999999999999999990"));

  // Large positive * small negative
  CHECK_EQ(bigint("1000000000000000000") * bigint(-1),
           bigint("-1000000000000000000"));

  // Multiplication with very small and very large numbers
  CHECK_EQ(bigint(1) * bigint("999999999999999999"),
           bigint("999999999999999999"));
  CHECK_EQ(bigint("-1") * bigint("999999999999999999"),
           bigint("-999999999999999999"));
}
#endif

// minus operator

inline bigint bigint::operator-(const bigint &b) const noexcept {
  bigint res;
  res += *this;
  res -= b;
  return res;
}

inline const bigint &bigint::operator-=(const bigint &b) noexcept {
  *this += -b;
  return *this;
}

inline bigint bigint::operator-() const noexcept {
  bigint res;
  res += *this;
  if (!is_zero()) {
    res.sign = !res.sign;
  }
  return res;
}

#ifdef DOCTEST_LIBRARY_INCLUDED
TEST_CASE("operator- basic functionality") {
  // Subtraction resulting in zero
  CHECK_EQ(bigint(0) - bigint(0), bigint(0));
  CHECK_EQ(bigint(12345) - bigint(12345), bigint(0));
  CHECK_EQ(bigint(-12345) - bigint(-12345), bigint(0));

  // Subtracting zero
  CHECK_EQ(bigint(12345) - bigint(0), bigint(12345));
  CHECK_EQ(bigint(-12345) - bigint(0), bigint(-12345));
  CHECK_EQ(bigint(0) - bigint(12345), bigint(-12345));
  CHECK_EQ(bigint(0) - bigint(-12345), bigint(12345));

  // Positive number subtraction
  CHECK_EQ(bigint(12345) - bigint(54321), bigint(-41976));
  CHECK_EQ(bigint(54321) - bigint(12345), bigint(41976));

  // Negative number subtraction
  CHECK_EQ(bigint(-12345) - bigint(54321), bigint(-66666));
  CHECK_EQ(bigint(12345) - bigint(-54321), bigint(66666));
  CHECK_EQ(bigint(-12345) - bigint(-54321), bigint(41976));
}

TEST_CASE("operator- edge cases") {
  // Subtraction resulting in negative numbers
  CHECK_EQ(bigint(12345) - bigint(54321), bigint(-41976));
  CHECK_EQ(bigint(-54321) - bigint(12345), bigint(-66666));

  // Subtraction involving large numbers
  CHECK_EQ(bigint("1000000000000000000") - bigint("999999999999999999"),
           bigint(1));
  CHECK_EQ(bigint("999999999999999999") - bigint("1000000000000000000"),
           bigint(-1));

  // Subtraction near the BASE boundary
  bigint base_minus_one("999999999999999999");
  CHECK_EQ(base_minus_one - bigint(1), bigint("999999999999999998"));
  CHECK_EQ(base_minus_one - bigint("1000000000000000000"), bigint(-1));
}

TEST_CASE("operator- chaining") {
  // Chained subtraction
  CHECK_EQ(bigint(10) - bigint(5) - bigint(2), bigint(3));
  CHECK_EQ(bigint(100) - bigint(50) - bigint(30), bigint(20));
  CHECK_EQ(bigint(-10) - bigint(-5) - bigint(-2), bigint(-3));
}

TEST_CASE("operator- extreme cases") {
  // Very large numbers
  CHECK_EQ(bigint("123456789123456789") - bigint("987654321987654321"),
           bigint("-864197532864197532"));
  CHECK_EQ(bigint("987654321987654321") - bigint("123456789123456789"),
           bigint("864197532864197532"));

  // Numbers with different signs
  CHECK_EQ(bigint("123456789123456789") - bigint("-987654321987654321"),
           bigint("1111111111111111110"));
  CHECK_EQ(bigint("-123456789123456789") - bigint("987654321987654321"),
           bigint("-1111111111111111110"));

  // Subtraction with small numbers
  CHECK_EQ(bigint(1) - bigint(1), bigint(0));
  CHECK_EQ(bigint(-1) - bigint(-1), bigint(0));
  CHECK_EQ(bigint(-1) - bigint(1), bigint(-2));
  CHECK_EQ(bigint(1) - bigint(-1), bigint(2));
}
#endif

// comparison operators

inline bool bigint::operator==(const bigint &b) const noexcept {
  return sign == b.sign && val == b.val;
}

#ifdef DOCTEST_LIBRARY_INCLUDED
TEST_CASE("equality") {
  CHECK_EQ(bigint(), bigint());

  CHECK_EQ(bigint(0), bigint(0));
  CHECK_EQ(bigint(0), bigint(-0));
  CHECK_EQ(bigint(12345), bigint(12345));
  CHECK_EQ(bigint(-12345), bigint(-12345));
  CHECK_EQ(bigint(123456789101112), bigint(123456789101112));

  CHECK_EQ(bigint("0"), bigint("0"));
  CHECK_EQ(bigint("0"), bigint("-0"));
  CHECK_EQ(bigint("12345"), bigint("12345"));
  CHECK_EQ(bigint("-12345"), bigint("-12345"));
  CHECK_EQ(bigint("1234567891011121314151617181920"),
           bigint("1234567891011121314151617181920"));
  CHECK_EQ(bigint("-1234567891011121314151617181920"),
           bigint("-1234567891011121314151617181920"));

  CHECK_EQ(bigint(0), bigint("0"));
  CHECK_EQ(bigint(0), bigint("-0"));
  CHECK_EQ(bigint("12345"), bigint(12345));
  CHECK_EQ(bigint(12345), bigint("12345"));
}
#endif

inline bool bigint::operator!=(const bigint &b) const noexcept {
  return sign != b.sign || val != b.val;
}

#ifdef DOCTEST_LIBRARY_INCLUDED
TEST_CASE("inequality") {
  CHECK_NE(bigint(12345), bigint(12346));
  CHECK_NE(bigint(12346), bigint(12345));
  CHECK_NE(bigint(12345), bigint(123456));
  CHECK_NE(bigint(12345), bigint(-12345));

  CHECK_NE(bigint("12345"), bigint("12346"));
  CHECK_NE(bigint("12346"), bigint("12345"));
  CHECK_NE(bigint("12345"), bigint("123456"));
  CHECK_NE(bigint("12345"), bigint("-12345"));
  CHECK_NE(bigint("-12345"), bigint("12345"));

  CHECK_NE(bigint("1234567891011121314151617181920"),
           bigint("1234567891011121314151617181921"));
  CHECK_NE(bigint("1234567891011121314151617181921"),
           bigint("1234567891011121314151617181920"));
  CHECK_NE(bigint("1234567891011121314151617181920"),
           bigint("-1234567891011121314151617181920"));
  CHECK_NE(bigint("-1234567891011121314151617181920"),
           bigint("1234567891011121314151617181920"));
  CHECK_NE(bigint("123456789101112131415161718192"),
           bigint("1234567891011121314151617181920"));

  CHECK_NE(bigint("12345"), bigint(12346));
  CHECK_NE(bigint(12345), bigint("12346"));
  CHECK_NE(bigint("12345"), bigint(123456));
  CHECK_NE(bigint("12345"), bigint(-12345));
  CHECK_NE(bigint(-12345), bigint("12345"));
}
#endif

inline bool bigint::operator<(const bigint &b) const noexcept {
  if (sign && !b.sign) // a is -ve and b is +ve
    return true;
  if (!sign && b.sign) // a is +ve and b is -ve
    return false;
  // both a and b are +ve
  if (!sign)
    return val_less(b);
  // both a and b are -ve
  return val_more(b);
}

#ifdef DOCTEST_LIBRARY_INCLUDED
TEST_CASE("less than") {
  CHECK_LT(bigint(12345), bigint(12346));
  CHECK_LT(bigint(12345), bigint(123456));
  CHECK_LT(bigint(-12345), bigint(12345));
  CHECK_LT(bigint(-123456), bigint(-12345));
  CHECK_LT(bigint(123456789101112), bigint(123456789101113));

  CHECK_LT(bigint("12345"), bigint("12346"));
  CHECK_LT(bigint("12345"), bigint("123456"));
  CHECK_LT(bigint("-12345"), bigint("12345"));
  CHECK_LT(bigint("-123456"), bigint("-12345"));

  CHECK_LT(bigint("1234567891011121314151617181920"),
           bigint("1234567891011121314151617181921"));
  CHECK_LT(bigint("-1234567891011121314151617181920"),
           bigint("1234567891011121314151617181920"));
  CHECK_LT(bigint("123456789101112131415161718192"),
           bigint("1234567891011121314151617181920"));

  CHECK_LT(bigint("12345"), bigint(12346));
  CHECK_LT(bigint(12345), bigint("12346"));
  CHECK_LT(bigint("12345"), bigint(123456));
  CHECK_LT(bigint(-12345), bigint("12345"));
}

TEST_CASE("less than false cases") {
  CHECK_FALSE(bigint(12346) < bigint(12345));
  CHECK_FALSE(bigint(123456) < bigint(12345));
  CHECK_FALSE(bigint(12345) < bigint(-12345));
  CHECK_FALSE(bigint(-12345) < bigint(-123456));
  CHECK_FALSE(bigint(123456789101113) < bigint(123456789101112));

  CHECK_FALSE(bigint("12346") < bigint("12345"));
  CHECK_FALSE(bigint("123456") < bigint("12345"));
  CHECK_FALSE(bigint("12345") < bigint("-12345"));
  CHECK_FALSE(bigint("-12345") < bigint("-123456"));

  CHECK_FALSE(bigint("1234567891011121314151617181921") <
              bigint("1234567891011121314151617181920"));
  CHECK_FALSE(bigint("1234567891011121314151617181920") <
              bigint("-1234567891011121314151617181920"));
  CHECK_FALSE(bigint("1234567891011121314151617181920") <
              bigint("123456789101112131415161718192"));

  CHECK_FALSE(bigint(12346) < bigint("12345"));
  CHECK_FALSE(bigint("12346") < bigint(12345));
  CHECK_FALSE(bigint(123456) < bigint("12345"));
  CHECK_FALSE(bigint("12345") < bigint(-12345));
}
#endif

inline bool bigint::operator>(const bigint &b) const noexcept {
  if (sign && !b.sign) // a is -ve and b is +ve
    return false;
  if (!sign && b.sign) // a is +ve and b is -ve
    return true;
  // both a and b are +ve
  if (!sign)
    return val_more(b);
  // both a and b are -ve
  return val_less(b);
}

#ifdef DOCTEST_LIBRARY_INCLUDED
TEST_CASE("more than") {
  CHECK_GT(bigint(12346), bigint(12345));
  CHECK_GT(bigint(123456), bigint(12345));
  CHECK_GT(bigint(12345), bigint(-12345));
  CHECK_GT(bigint(-12345), bigint(-123456));
  CHECK_GT(bigint(123456789101113), bigint(123456789101112));

  CHECK_GT(bigint("12346"), bigint("12345"));
  CHECK_GT(bigint("123456"), bigint("12345"));
  CHECK_GT(bigint("12345"), bigint("-12345"));
  CHECK_GT(bigint("-12345"), bigint("-123456"));

  CHECK_GT(bigint("1234567891011121314151617181921"),
           bigint("1234567891011121314151617181920"));
  CHECK_GT(bigint("1234567891011121314151617181920"),
           bigint("-1234567891011121314151617181920"));
  CHECK_GT(bigint("1234567891011121314151617181920"),
           bigint("123456789101112131415161718192"));

  CHECK_GT(bigint(12346), bigint("12345"));
  CHECK_GT(bigint("12346"), bigint(12345));
  CHECK_GT(bigint(123456), bigint("12345"));
  CHECK_GT(bigint("12345"), bigint(-12345));
}

TEST_CASE("more than false cases") {
  CHECK_FALSE(bigint(12345) > bigint(12346));
  CHECK_FALSE(bigint(12345) > bigint(123456));
  CHECK_FALSE(bigint(-12345) > bigint(12345));
  CHECK_FALSE(bigint(-123456) > bigint(-12345));
  CHECK_FALSE(bigint(123456789101112) > bigint(123456789101113));

  CHECK_FALSE(bigint("12345") > bigint("12346"));
  CHECK_FALSE(bigint("12345") > bigint("123456"));
  CHECK_FALSE(bigint("-12345") > bigint("12345"));
  CHECK_FALSE(bigint("-123456") > bigint("-12345"));

  CHECK_FALSE(bigint("1234567891011121314151617181920") >
              bigint("1234567891011121314151617181921"));
  CHECK_FALSE(bigint("-1234567891011121314151617181920") >
              bigint("1234567891011121314151617181920"));
  CHECK_FALSE(bigint("123456789101112131415161718192") >
              bigint("1234567891011121314151617181920"));

  CHECK_FALSE(bigint("12345") > bigint(12346));
  CHECK_FALSE(bigint(12345) > bigint("12346"));
  CHECK_FALSE(bigint("12345") > bigint(123456));
  CHECK_FALSE(bigint(-12345) > bigint("12345"));
}
#endif

inline bool bigint::operator<=(const bigint &b) const noexcept {
  return !(*this > b);
}

inline bool bigint::operator>=(const bigint &b) const noexcept {
  return !(*this < b);
}

inline std::ostream &operator<<(std::ostream &os, const bigint &a) noexcept {
  if (a.sign)
    os << '-';
  for (const auto &ai : a.val | std::views::reverse) {
    os << ai;
  }
  return os;
}

inline void bigint::val_plus(const bigint &b, std::size_t offset) noexcept {
  if (b.val.size() + offset > val.size())
    val.resize(b.val.size() + offset, 0);
  WORD carry = 0;
  for (std::size_t i = offset; i < val.size(); i++) {
    const WORD ai = val[i];
    const WORD bi = i - offset < b.val.size() ? b.val[i - offset] : 0;
    val[i] = (ai + bi + carry) % BASE;
    carry = (ai + bi + carry) / BASE;
  }
  if (carry) {
    val.push_back(carry);
  }
}

inline void bigint::val_monus(const bigint &b) noexcept {
  WORD carry = 0;
  for (std::size_t i = 0; i < val.size(); i++) {
    const WORD ai = val[i];
    const WORD bi = i < b.val.size() ? b.val[i] : 0;
    val[i] = (ai - bi - carry + BASE) % BASE;
    carry = (ai - bi - carry < 0) ? 1 : 0;
  }
  while (val.size() > 1 && !val.back()) {
    val.pop_back();
  }
}

inline void bigint::val_mult(const bigint &b) noexcept {
  bigint s(0);
  for (std::size_t i = 0; i < b.val.size(); i++) {
    s.val_plus(*this * b.val[i], i);
  }
  val = s.val;
  while (val.size() > 1 && !val.back()) {
    val.pop_back();
  }
}

inline bool bigint::val_less(const bigint &b) const noexcept {
  if (val.size() < b.val.size())
    return true;
  if (val.size() > b.val.size())
    return false;
  for (const auto [ai, bi] :
       std::views::zip(val, b.val) | std::views::reverse) {
    if (ai != bi)
      return ai < bi;
  }
  return false;
}

inline bool bigint::val_more(const bigint &b) const noexcept {
  if (val.size() < b.val.size())
    return false;
  if (val.size() > b.val.size())
    return true;
  for (const auto [ai, bi] :
       std::views::zip(val, b.val) | std::views::reverse) {
    if (ai != bi)
      return ai > bi;
  }
  return false;
}

inline bool bigint::is_zero() const noexcept {
  return val.size() == 1 && !val.at(0);
}
