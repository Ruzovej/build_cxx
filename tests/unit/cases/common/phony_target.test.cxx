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

#include "build_cxx/common/phony_target.hxx"

#include <doctest/doctest.h>

#include "build_cxx/client/core.hxx"
#include "build_cxx/test_helpers/test_phony_target.hxx"

namespace build_cxx::common {
namespace {

TEST_CASE("phony_target") {
  project test_project{"pttp", "0.1.0", __FILE__};

  BUILD_CXX_DEFINE_LOCATION(loc, location::no_index);
  // testing it on single "isolated" target should be enough:
  BUILD_CXX_DEFINE_DEPS_ARRAY(deps_arr, deps_n);

  test_phony_target pt{&loc, true, "tpt", deps_arr, deps_n};

  test_project.add_target(&pt);

  REQUIRE(pt.resolved_kind.empty());
  REQUIRE(pt.resolved_name.empty());

  REQUIRE_NOTHROW(pt.resolve_own_traits());

  REQUIRE_EQ(pt.resolved_kind, phony_target::kind);
  REQUIRE_EQ(pt.resolved_name,
             std::string{test_project.name} + "::" + std::string{pt.name});
  REQUIRE_EQ(pt.resolved_name,
             phony_target::resolve_name(test_project.name, pt.name));

  // TODO test `last_modification_time` ...
}

} // namespace
} // namespace build_cxx::common
