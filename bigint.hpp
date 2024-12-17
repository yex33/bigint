#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
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

private:
  static constexpr WORD WORD_MAX = std::numeric_limits<WORD>::max();
  static constexpr DBLWORD BASE = static_cast<DBLWORD>(WORD_MAX) + 1;

  // sign == true if n is negative, sign == false if n is positive
  bool sign;
  std::vector<WORD> n;

public:
  bigint() noexcept : sign(false), n{0} {};
  /**
   * a constructor that takes a string and a base, and converts the string to an
   * arbitrary-precision integer in that base.
   * @param sv the string to convert
   * @param base the number base
   */
  bigint(std::string_view sv, int base = 10) noexcept(false);

  [[nodiscard]]
  bigint operator+(const WORD b) const noexcept;
  const bigint &operator+=(const WORD b) noexcept;
  [[nodiscard]]
  bigint operator*(const WORD b) const noexcept;
  const bigint &operator*=(const WORD b) noexcept;

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
};

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

inline bigint bigint::operator+(const WORD b) const noexcept {
  bigint res;
  res += *this;
  res += b;
  return res;
}

inline const bigint &bigint::operator+=(const WORD b) noexcept {
  WORD carry = b;
  for (WORD &ai : n) {
    WORD next = (carry > WORD_MAX - ai) ? 1 : 0;
    ai += carry;
    carry = next;
    if (!carry)
      break;
  }
  if (carry > 0) {
    n.push_back(carry);
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
  if (n.size() == 1 && !n[0])
    return *this;
  if (!b) {
    n.clear();
    n.push_back(0);
    sign = false;
    return *this;
  }
  WORD carry = 0;
  for (WORD &ai : n) {
    DBLWORD prod = static_cast<DBLWORD>(ai) * static_cast<DBLWORD>(b);
    ai = static_cast<WORD>(prod % BASE + carry);
    carry = static_cast<WORD>(prod / BASE);
  }
  if (carry > 0) {
    n.push_back(carry);
  }
  return *this;
}

inline bigint bigint::operator+(const bigint &b) const noexcept {}

inline const bigint &bigint::operator+=(const bigint &b) noexcept {}

inline bigint bigint::operator-() const noexcept {
  bigint res = *this;
  res.sign = !res.sign;
  return res;
}
