/*
  Copyright 2026 Lukáš Růžička

  This file is part of build_cxx.

  build_cxx is free software: you can redistribute it and/or modify it under the
  terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your option) any
  later version.

  build_cxx is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License along
  with build_cxx. If not, see <https://www.gnu.org/licenses/>.
*/

#include "build_cxx/common/target_status.hxx"

#include <doctest/doctest.h>

namespace build_cxx {
namespace {

constexpr common::target_status uninitialized() {
  return common::target_status{};
}

constexpr common::target_status needs_update() {
  return common::target_status{common::target_status::needs_update};
}

constexpr common::target_status file_mod_time(
    common::target_status::file_modification_time_t const mod_time = 42) {
  return common::target_status{mod_time};
}

TEST_CASE("common::target_status") {
  [[maybe_unused]] bool discarded;

  SUBCASE("certainly_needs_update()") {
    SUBCASE("uninitialized") {
      auto const ts{uninitialized()};

      REQUIRE_THROWS(discarded = ts.certainly_needs_update());
    }

    SUBCASE("needs update") {
      auto const ts{needs_update()};

      REQUIRE(ts.certainly_needs_update());
    }

    SUBCASE("file mod time") {
      auto const ts{file_mod_time()};

      REQUIRE(!ts.certainly_needs_update());
    }
  }

  SUBCASE("merge_with()") {
    SUBCASE("first uninitialized") {
      auto ts{uninitialized()};

      SUBCASE("second uninitialized") {
        REQUIRE_THROWS(ts.merge_with(uninitialized()));
      }

      SUBCASE("second needs update") {
        REQUIRE_NOTHROW(ts.merge_with(needs_update()));
        REQUIRE(ts.certainly_needs_update()); // for example ...
      }

      SUBCASE("second file mod time") {
        REQUIRE_NOTHROW(ts.merge_with(file_mod_time()));
        REQUIRE(!ts.certainly_needs_update()); // for example ...
      }
    }

    SUBCASE("first needs update") {
      auto ts{needs_update()};

      SUBCASE("second uninitialized") {
        REQUIRE_THROWS(ts.merge_with(uninitialized()));
      }

      SUBCASE("second needs update") {
        REQUIRE_NOTHROW(ts.merge_with(needs_update()));
        REQUIRE(ts.certainly_needs_update()); // for example ...
      }

      SUBCASE("second file mod time") {
        REQUIRE_NOTHROW(ts.merge_with(file_mod_time()));
        REQUIRE(ts.certainly_needs_update()); // for example ...
      }
    }

    SUBCASE("first file mod time") {
      auto ts{file_mod_time()};

      SUBCASE("second uninitialized") {
        REQUIRE_THROWS(ts.merge_with(uninitialized()));
      }

      SUBCASE("second needs update") {
        REQUIRE_NOTHROW(ts.merge_with(needs_update()));
        REQUIRE(ts.certainly_needs_update()); // for example ...
      }

      SUBCASE("second file mod time") {
        REQUIRE_NOTHROW(ts.merge_with(file_mod_time()));
        REQUIRE(!ts.certainly_needs_update()); // for example ...
      }
    }
  }

  SUBCASE("needs_update_compared_to()") {
    SUBCASE("first uninitialized") {
      auto ts{uninitialized()};

      SUBCASE("second uninitialized") {
        REQUIRE_THROWS(discarded =
                           ts.needs_update_compared_to(uninitialized()));
      }

      SUBCASE("second needs update") {
        REQUIRE_THROWS(discarded =
                           ts.needs_update_compared_to(uninitialized()));
      }

      SUBCASE("second file mod time") {
        REQUIRE_THROWS(discarded =
                           ts.needs_update_compared_to(uninitialized()));
      }
    }

    SUBCASE("first needs update") {
      auto ts{needs_update()};

      SUBCASE("second uninitialized") {
        REQUIRE(ts.needs_update_compared_to(uninitialized()));
      }

      SUBCASE("second needs update") {
        REQUIRE(ts.needs_update_compared_to(needs_update()));
      }

      SUBCASE("second file mod time") {
        REQUIRE(ts.needs_update_compared_to(file_mod_time()));
      }
    }

    SUBCASE("first file mod time") {
      using mod_time_t = common::target_status::file_modification_time_t;

      static mod_time_t constexpr sooner{1};
      static mod_time_t constexpr now{2};
      static mod_time_t constexpr later{3};

      auto ts{file_mod_time(now)};

      SUBCASE("second uninitialized") {
        REQUIRE(!ts.needs_update_compared_to(uninitialized()));
      }

      SUBCASE("second needs update") {
        REQUIRE(ts.needs_update_compared_to(needs_update()));
      }

      SUBCASE("second file mod time") {
        static_assert((sooner < now) && (now < later));

        SUBCASE("second modified later") {
          REQUIRE(ts.needs_update_compared_to(file_mod_time(later)));
        }

        SUBCASE("second modified at the same time") {
          REQUIRE(!ts.needs_update_compared_to(file_mod_time(now)));
        }

        SUBCASE("second modified sooner") {
          REQUIRE(!ts.needs_update_compared_to(file_mod_time(sooner)));
        }
      }
    }
  }
}

} // namespace
} // namespace build_cxx
