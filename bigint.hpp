#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
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
  using DBLWORD = std::uint64_t;
  using WORD = std::uint32_t;

protected:
  // sign == true if n is negative, sign == false if n is positive
  bool sign;
  std::vector<WORD> val;

public:
  bigint() noexcept : sign(false), val{0} {};
  bigint(int64_t n) noexcept;
  /**
   * a constructor that takes a string and a base, and converts the string to an
   * arbitrary-precision integer in that base.
   * @param sv the string to convert
   * @param base the number base
   */
  bigint(std::string_view sv, int base = 10) noexcept(false);

  [[nodiscard]]
  const bigint operator+(const WORD b) const noexcept;
  const bigint &operator+=(const WORD b) noexcept;
  [[nodiscard]]
  const bigint operator*(const WORD b) const noexcept;
  const bigint &operator*=(const WORD b) noexcept;

  [[nodiscard]]
  const bigint operator+(const bigint &b) const noexcept;
  const bigint &operator+=(const bigint &b) noexcept;
  [[nodiscard]]
  const bigint operator-(const bigint &b) const noexcept;
  const bigint &operator-=(const bigint &b) noexcept;
  [[nodiscard]]
  const bigint operator*(const bigint &b) const noexcept;
  const bigint &operator*=(const bigint &b) noexcept;

  [[nodiscard]]
  const bigint operator-() const noexcept;

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

private:
  static constexpr WORD WORD_MAX = std::numeric_limits<WORD>::max();
  static constexpr DBLWORD BASE = static_cast<DBLWORD>(WORD_MAX) + 1;

  const bigint &val_plus(const bigint &b) noexcept;
  const bigint &val_minus(const bigint &b) noexcept;
  [[nodiscard]]
  bool val_less(const bigint &b) const noexcept;
  [[nodiscard]]
  bool val_more(const bigint &b) const noexcept;
};

inline bigint::bigint(int64_t n) noexcept : sign(false) {
  if (n < 0)
    sign = true;
  n = std::abs(n);
  WORD chunk = n & WORD_MAX;
  while (chunk > 0) {
    val.push_back(chunk);
    n = n >> (sizeof(WORD) * 8);
    chunk = n & WORD_MAX;
  }
}

inline bigint::bigint(std::string_view sv, int base) noexcept(false)
    : bigint() {
  bool minus_flg = false, alnum_flg = false;
  for (char ch : sv) {
    if (std::isalnum(ch)) {
      alnum_flg = true;
      if (std::toupper(ch) - '0' >= base) {
        std::stringstream ss;
        ss << "invalid character " << std::quoted(std::string{ch})
           << " used for integer of base " << base;
        throw std::invalid_argument(ss.str());
      }
    } else if (ch == '-' && !alnum_flg && !minus_flg) {
      sign = true;
      minus_flg = true;
    } else {
      std::stringstream ss;
      ss << "invalid character " << std::quoted(std::string{ch}) << " found";
      throw std::invalid_argument(ss.str());
    }
  }
  if (minus_flg) {
    sv = sv.substr(1);
  }

  std::size_t wnd_size =
      static_cast<std::size_t>(std::log(WORD_MAX) / std::log(base));

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

inline const bigint bigint::operator+(const WORD b) const noexcept {
  bigint res;
  res += *this;
  res += b;
  return res;
}

inline const bigint &bigint::operator+=(const WORD b) noexcept {
  WORD carry = b;
  for (WORD &ai : val) {
    WORD next = (ai + carry < ai) ? 1 : 0;
    ai += carry;
    carry = next;
    if (!carry)
      break;
  }
  if (carry > 0) {
    val.push_back(carry);
  }
  return *this;
}

inline const bigint bigint::operator*(const WORD b) const noexcept {
  bigint res;
  res += *this;
  res *= b;
  return res;
}

inline const bigint &bigint::operator*=(const WORD b) noexcept {
  if (val.size() == 1 && !val[0])
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

inline const bigint bigint::operator+(const bigint &b) const noexcept {}

inline const bigint &bigint::operator+=(const bigint &b) noexcept {
  if (sign == b.sign) {
    
  }
}

inline const bigint bigint::operator-() const noexcept {
  bigint res = *this;
  res.sign = !res.sign;
  return res;
}

inline bool bigint::operator==(const bigint &b) const noexcept {
  return sign == b.sign && val == b.val;
}

inline bool bigint::operator!=(const bigint &b) const noexcept {
  return sign != b.sign || val != b.val;
}

inline bool bigint::operator<(const bigint &b) const noexcept {
  if (sign && !b.sign) // a is -ve and b is +ve
    return true;
  if (!sign && b.sign) // a is +ve and b is -ve
    return false;
  // both a and b are +ve
  return val_less(b);
}

inline bool bigint::operator>(const bigint &b) const noexcept {
  if (sign && !b.sign) // a is -ve and b is +ve
    return false;
  if (!sign && b.sign) // a is +ve and b is -ve
    return true;
  // both a and b are +ve
  return val_more(b);
}

inline bool bigint::operator<=(const bigint &b) const noexcept { return !(*this > b); }

inline bool bigint::operator>=(const bigint &b) const noexcept { return !(*this < b); }

inline bool bigint::val_less(const bigint &b) const noexcept {
  if (val.size() < b.val.size())
    return true;
  if (val.size() > b.val.size())
    return false;
  for (const auto [ai, bi] : std::views::zip(val, b.val) | std::views::reverse) {
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
  for (const auto [ai, bi] : std::views::zip(val, b.val) | std::views::reverse) {
    if (ai > bi)
      return true;
  }
  return false;
}
