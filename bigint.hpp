#include <cstdint>
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
  }
}

#ifdef DOCTEST_LIBRARY_INCLUDED

TEST_CASE("[bigint] reeee") {
  CHECK(1 == 1);
}


#endif
