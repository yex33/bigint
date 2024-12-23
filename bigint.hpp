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
  // val holds each digit of bigint
  std::vector<WORD> val;

public:
  /**
   * Default constructor for bigint. Initializes to 0.
   */
  bigint() noexcept : sign(false), val{0} {};

  /**
   * Constructs a bigint from a 64-bit integer.
   * @param n The integer to initialize the bigint.
   */
  explicit bigint(int64_t n) noexcept;

  /**
   * A constructor that takes a string and converts the string to an
   * arbitrary-precision integer.
   * @param sv the string to convert
   */
  explicit bigint(std::string_view sv, int base = 10) noexcept(false);

  /**
   * Adds a single WORD to the bigint.
   * @param b The WORD to add.
   * @return A new bigint representing the result.
   */
  [[nodiscard]]
  bigint operator+(WORD b) const noexcept;

  /**
   * Adds a single WORD to the bigint in-place.
   * @param b The WORD to add.
   * @return A reference to the updated bigint.
   */
  const bigint &operator+=(WORD b) noexcept;

  /**
   * Multiplies the bigint by a single WORD.
   * @param b The WORD to multiply with.
   * @return A new bigint representing the result.
   */
  [[nodiscard]]
  bigint operator*(WORD b) const noexcept;

  /**
   * Multiplies the bigint by a single WORD in-place.
   * @param b The WORD to multiply with.
   * @return A reference to the updated bigint.
   */
  const bigint &operator*=(WORD b) noexcept;

  /**
   * Adds another bigint to this bigint.
   * @param b The bigint to add.
   * @return A new bigint representing the result.
   */
  [[nodiscard]]
  bigint operator+(const bigint &b) const noexcept;

  /**
   * Adds another bigint to this bigint in-place.
   * @param b The bigint to add.
   * @return A reference to the updated bigint.
   */
  const bigint &operator+=(const bigint &b) noexcept;

  /**
   * Subtracts another bigint from this bigint.
   * @param b The bigint to subtract.
   * @return A new bigint representing the result.
   */
  [[nodiscard]]
  bigint operator-(const bigint &b) const noexcept;

  /**
   * Subtracts another bigint from this bigint in-place.
   * @param b The bigint to subtract.
   * @return A reference to the updated bigint.
   */
  const bigint &operator-=(const bigint &b) noexcept;

  /**
   * Multiplies this bigint by another bigint.
   * @param b The bigint to multiply with.
   * @return A new bigint representing the result.
   */
  [[nodiscard]]
  bigint operator*(const bigint &b) const noexcept;

  /**
   * Multiplies this bigint by another bigint in-place.
   * @param b The bigint to multiply with.
   * @return A reference to the updated bigint.
   */
  const bigint &operator*=(const bigint &b) noexcept;

  /**
   * Computes the negation of this bigint.
   * @return A new bigint representing the negated value.
   */
  [[nodiscard]]
  bigint operator-() const noexcept;

  /**
   * Checks equality between two bigints.
   * @param b The bigint to compare with.
   * @return True if the two bigints are equal, false otherwise.
   */
  [[nodiscard]]
  bool operator==(const bigint &b) const noexcept;

  /**
   * Checks inequality between two bigints.
   * @param b The bigint to compare with.
   * @return True if the two bigints are not equal, false otherwise.
   */
  [[nodiscard]]
  bool operator!=(const bigint &b) const noexcept;

  /**
   * Checks if this bigint is less than another bigint.
   * @param b The bigint to compare with.
   * @return True if this bigint is less than the given bigint, false otherwise.
   */
  [[nodiscard]]
  bool operator<(const bigint &b) const noexcept;

  /**
   * Checks if this bigint is greater than another bigint.
   * @param b The bigint to compare with.
   * @return True if this bigint is greater than the given bigint, false
   * otherwise.
   */
  [[nodiscard]]
  bool operator>(const bigint &b) const noexcept;

  /**
   * Checks if this bigint is less than or equal to another bigint.
   * @param b The bigint to compare with.
   * @return True if this bigint is less than or equal to the given bigint,
   * false otherwise.
   */
  [[nodiscard]]
  bool operator<=(const bigint &b) const noexcept;

  /**
   * Checks if this bigint is greater than or equal to another bigint.
   * @param b The bigint to compare with.
   * @return True if this bigint is greater than or equal to the given bigint,
   * false otherwise.
   */
  [[nodiscard]]
  bool operator>=(const bigint &b) const noexcept;

  /**
   * Prefix increment operator.
   * @return A reference to the updated bigint.
   */
  [[nodiscard]]
  const bigint &operator++() noexcept;

  /**
   * Postfix increment operator.
   * @return A new bigint representing the value before the increment.
   */
  [[nodiscard]]
  bigint operator++(int) noexcept;

  /**
   * Prefix decrement operator.
   * @return A reference to the updated bigint.
   */
  [[nodiscard]]
  const bigint &operator--() noexcept;

  /**
   * Postfix decrement operator.
   * @return A new bigint representing the value before the decrement.
   */
  [[nodiscard]]
  bigint operator--(int) noexcept;

  /**
   * Outputs the bigint to the given output stream.
   * @param os The output stream.
   * @param a The bigint to output.
   * @return The output stream.
   */
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
TEST_CASE("[bigint] constructor") {
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
TEST_CASE("[bigint] additions") {
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

  // Addition resulting in zero
  CHECK_EQ(bigint(12345) + bigint(-12345), bigint(0));
  CHECK_EQ(bigint(-12345) + bigint(12345), bigint(0));

  // Large number addition
  CHECK_EQ(bigint("1234567891011121314151617181920") +
               bigint("987654321098765432101234567890"),
           bigint("2222222212109886746252851749810"));
  CHECK_EQ(bigint("999999999999999999999999999999") + bigint("1"),
           bigint("1000000000000000000000000000000"));
}

TEST_CASE("[bigint] addition chaining") {
  CHECK_EQ(bigint(12345) + bigint(67890) + bigint(11111), bigint(91346));
  CHECK_EQ(bigint(12345) + bigint(-12345) + bigint(67890), bigint(67890));
  CHECK_EQ(bigint(12345) + bigint(0) + bigint(-12345), bigint(0));
}

TEST_CASE("[bigint] addition edge cases") {
  CHECK_EQ(bigint("123456789123456789123456789") +
               bigint("987654321987654321987654321"),
           bigint("1111111111111111111111111110"));
  CHECK_EQ(bigint("-123456789123456789123456789") +
               bigint("987654321987654321987654321"),
           bigint("864197532864197532864197532"));
  CHECK_EQ(bigint("-1000000000000000000000000") + bigint(1),
           bigint("-999999999999999999999999"));
  CHECK_EQ(bigint("1000000000000000000000000") + bigint(-1),
           bigint("999999999999999999999999"));
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
TEST_CASE("[bigint] multiplications") {
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

TEST_CASE("[bigint] multiplication chaining") {
  CHECK_EQ(bigint(2) * bigint(3) * bigint(4), bigint(24));
  CHECK_EQ(bigint(10) * bigint(-5) * bigint(2), bigint(-100));
  CHECK_EQ(bigint(-2) * bigint(-3) * bigint(-4), bigint(-24));
}

TEST_CASE("[bigint] multiplication edge cases") {
  CHECK_EQ(bigint("123456789123456789123456789123456789") *
               bigint("987654321987654321987654321987654321"),
           bigint("121932631356500531591068431825636331816338969581771069347203"
                  "169112635269"));
  CHECK_EQ(bigint("1000000000000000000") * bigint(-1),
           bigint("-1000000000000000000"));
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

// pre/post-fix operators
inline const bigint &bigint::operator++() noexcept {
  *this += 1;
  return *this;
}

inline bigint bigint::operator++(int) noexcept {
  bigint res;
  res += *this;
  *this += 1;
  return res;
}

inline const bigint &bigint::operator--() noexcept {
  *this -= bigint(1);
  return *this;
}

inline bigint bigint::operator--(int) noexcept {
  bigint res;
  res += *this;
  *this -= bigint(1);
  return res;
}

#ifdef DOCTEST_LIBRARY_INCLUDED
TEST_CASE("[bigint] subtractions") {
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

TEST_CASE("[bigint] subtraction chaining") {
  CHECK_EQ(bigint(10) - bigint(5) - bigint(2), bigint(3));
  CHECK_EQ(bigint(100) - bigint(50) - bigint(30), bigint(20));
  CHECK_EQ(bigint(-10) - bigint(-5) - bigint(-2), bigint(-3));
}

TEST_CASE("[bigint] subtraction edge cases") {
  CHECK_EQ(bigint("123456789123456789") - bigint("987654321987654321"),
           bigint("-864197532864197532"));
  CHECK_EQ(bigint("987654321987654321") - bigint("123456789123456789"),
           bigint("864197532864197532"));
  CHECK_EQ(bigint("123456789123456789") - bigint("-987654321987654321"),
           bigint("1111111111111111110"));
  CHECK_EQ(bigint("-123456789123456789") - bigint("987654321987654321"),
           bigint("-1111111111111111110"));
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
TEST_CASE("[bigint] equality") {
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
TEST_CASE("[bigint] inequality") {
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
TEST_CASE("[bigint] less than") {
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

TEST_CASE("[bigint] less than false cases") {
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
TEST_CASE("[bigint] more than") {
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

TEST_CASE("[bigint] more than false cases") {
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
