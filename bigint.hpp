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
  using WORKUINT = std::uint64_t;
  using RESUINT = std::uint32_t;

private:
  static constexpr RESUINT RES_UMAX = std::numeric_limits<RESUINT>::max();
  static constexpr WORKUINT BASE = static_cast<WORKUINT>(RES_UMAX) + 1;
  bool sign;
  std::vector<RESUINT> n;

public:
  bigint() noexcept : sign(false), n(std::vector<RESUINT>(1, 0)) {};
  /**
   * a constructor that takes a string and a base, and converts the string to an
   * arbitrary-precision integer in that base.
   * @param sv the string to convert
   * @param base the number base
   */
  bigint(std::string_view sv, std::int32_t base = 10) noexcept(false);

  bigint operator+(const RESUINT b) const noexcept;
  const bigint &operator+=(const RESUINT b) noexcept;
  bigint operator*(const RESUINT b) const noexcept;
  const bigint &operator*=(const RESUINT b) noexcept;

  bigint operator+(const bigint &b) const noexcept;
  const bigint &operator+=(const bigint &b) noexcept;
  bigint operator*(const bigint &b) const noexcept;
  const bigint &operator*=(const bigint &b) noexcept;
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

  std::size_t wnd_size = static_cast<std::size_t>(std::log(RES_UMAX) / std::log(base));

  std::size_t offset = sv.size() % wnd_size;
  std::string_view sub = sv.substr(0, offset);
  RESUINT wnd;
  std::from_chars(sub.data(), sub.data() + sub.size(), wnd, base);
  *this += wnd;
  for (std::size_t pos = offset; pos < sv.size(); pos += wnd_size) {
    sub = sv.substr(pos, wnd_size);
    std::from_chars(sub.data(), sub.data() + sub.size(), wnd, base);
    *this *= static_cast<RESUINT>(std::pow(base, wnd_size));
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

inline bigint bigint::operator+(const RESUINT b) const noexcept {
  bigint res;
  res += *this;
  res += b;
  return res;
}

inline const bigint &bigint::operator+=(const RESUINT b) noexcept {
  if (b > RES_UMAX - n[0]) {
    if (n.size() == 1) {
      n.push_back(1);
    } else {
      n[1] += 1;
    }
    n[0] += b;
  }
  return *this;
}

inline bigint bigint::operator*(const RESUINT b) const noexcept {
  bigint res;
  res += *this;
  res *= b;
  return res;
}

inline const bigint &bigint::operator*=(const RESUINT b) noexcept {
  WORKUINT carry = 0;
  for (RESUINT &ai : n) {
    WORKUINT prod = static_cast<WORKUINT>(ai) * static_cast<WORKUINT>(b);
    ai = static_cast<RESUINT>(prod % BASE + carry);
    carry = prod / BASE;
  }
  if (carry > 0) {
    n.push_back(static_cast<RESUINT>(carry));
  }
  return *this;
}
