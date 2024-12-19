#pragma once

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef DOCTEST
#include "doctest.h"
#endif

class bigint {
  using DBLWORD = std::uint64_t;
  using WORD = std::uint32_t;

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
  const bigint operator-(const bigint &b) const noexcept;
  const bigint &operator-=(const bigint &b) noexcept;
  [[nodiscard]]
  const bigint operator*(const bigint &b) const noexcept;
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
  static constexpr WORD WORD_MAX = std::numeric_limits<WORD>::max();
  static constexpr DBLWORD BASE = static_cast<DBLWORD>(WORD_MAX) + 1;

  const bigint &val_plus(const bigint &b) noexcept;
  const bigint &val_monus(const bigint &b) noexcept;
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
  WORD chunk = static_cast<WORD>(n & WORD_MAX);
  while (chunk > 0) {
    val.push_back(chunk);
    n = n >> (sizeof(WORD) * 8);
    chunk = static_cast<WORD>(n & WORD_MAX);
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

  auto wnd_size = static_cast<std::size_t>(std::log(WORD_MAX) / std::log(base));

  std::size_t offset = sv.size() % wnd_size;
  std::string_view sub = sv.substr(0, offset);
  WORD wnd;
  std::from_chars(sub.data(), sub.data() + sub.size(), wnd, base);
  *this += wnd;
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

inline bigint bigint::operator+(const WORD b) const noexcept {
  bigint res;
  res += *this;
  res += b;
  return res;
}

inline const bigint &bigint::operator+=(const WORD b) noexcept {
  WORD carry = b;
  for (std::size_t i = 0; i < val.size(); i++) {
    WORD ai = val[i];
    val[i] = ai + carry;
    carry = (val[i] < ai) ? 1u : 0;
    if (!carry)
      break;
  }
  if (carry > 0) {
    val.push_back(carry);
  }
  return *this;
}

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
    DBLWORD prod = static_cast<DBLWORD>(ai) * static_cast<DBLWORD>(b);
    ai = static_cast<WORD>(prod % BASE + carry);
    carry = static_cast<WORD>(prod / BASE);
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
           bigint("2222222222119886746252851749810"));
  CHECK_EQ(bigint("999999999999999999999999999999") + bigint("1"),
           bigint("1000000000000000000000000000000"));

  // Adding a WORD to bigint
  CHECK_EQ(bigint(12345) + static_cast<std::uint32_t>(67890), bigint(80235));
  CHECK_EQ(bigint("999999999999999999999999") + static_cast<std::uint32_t>(1),
           bigint("1000000000000000000000000"));
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

inline bigint bigint::operator-() const noexcept {
  bigint res = *this;
  res.sign = !res.sign;
  return res;
}

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
#endif

inline bool bigint::operator<=(const bigint &b) const noexcept {
  return !(*this > b);
}

inline bool bigint::operator>=(const bigint &b) const noexcept {
  return !(*this < b);
}

inline std::ostream &operator<<(std::ostream &os, const bigint &a) noexcept {
  os << '{';
  if (a.sign)
    os << '-';
  for (const auto &ai : a.val) {
    os << ai << ", ";
  }
  os << '}';
  return os;
}

inline const bigint &bigint::val_plus(const bigint &b) noexcept {
  if (b.val.size() > val.size())
    val.resize(b.val.size(), 0);
  WORD carry = 0;
  for (std::size_t i = 0; i < val.size(); i++) {
    const WORD ai = val[i];
    const WORD bi = i < b.val.size() ? b.val[i] : 0;
    val[i] = ai + bi + carry;
    carry = (val[i] < ai) ? 1u : 0;
  }
  if (carry) {
    val.push_back(carry);
  }
  return *this;
}

inline const bigint &bigint::val_monus(const bigint &b) noexcept {
  WORD carry = 0;
  for (std::size_t i = 0; i < val.size(); i++) {
    const WORD ai = val[i];
    const WORD bi = i < b.val.size() ? b.val[i] : 0;
    val[i] = ai - bi - carry;
    carry = (val[i] > ai) ? 1u : 0;
  }
  while (!*val.cend()) {
    val.pop_back();
  }
  return *this;
}

inline bool bigint::val_less(const bigint &b) const noexcept {
  if (val.size() < b.val.size())
    return true;
  if (val.size() > b.val.size())
    return false;
  for (const auto [ai, bi] :
       std::views::zip(val, b.val) | std::views::reverse) {
    if (ai < bi)
      return true;
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
    if (ai > bi)
      return true;
  }
  return false;
}

inline bool bigint::is_zero() const noexcept {
  return val.size() == 1 && !val.at(0);
}
