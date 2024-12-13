#include <cstdint>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#ifdef DOCTEST
#include "doctest.h"
#endif

class bigint {
  bool sign;
  std::vector<uint32_t> n;

public:
  bigint() : sign(false), n(std::vector<uint32_t>(1, 0)) {};
  bigint(std::string_view sv);
};

inline bigint::bigint(std::string_view sv) {
  for (auto it = sv.rbegin(); it != sv.rend(); it++) {
    if (*it == '-' && it != sv.rbegin()) {
      sign = true;
      continue;
    }
    uint32_t digit = static_cast<uint32_t>(std::stoul(std::string{*it}));
    std::cout << digit << std::endl;
  }
  for (char ch : sv | std::views::reverse) {

  }
}

#ifdef DOCTEST_LIBRARY_INCLUDED

TEST_CASE("[bigint] reeee") {
  bigint a("-42");
  CHECK(1 == 1);
}


#endif
