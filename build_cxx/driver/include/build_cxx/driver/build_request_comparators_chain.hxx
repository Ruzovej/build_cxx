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

#pragma once

#include <memory>
#include <queue>
#include <string_view>
#include <vector>

#include <build_cxx/common/fs_proxy.hxx>
#include <build_cxx/common/macros.h>

#include "build_cxx/driver/build_request.hxx"

namespace build_cxx::driver {

namespace sort_by {

static std::string_view constexpr name_asc{"name_asc"};
static std::string_view constexpr name_desc{"name_desc"};
static std::string_view constexpr mod_time_asc{"mod_time_asc"};
static std::string_view constexpr mod_time_desc{"mod_time_desc"};
static std::string_view constexpr exists{"exists"};
static std::string_view constexpr doesnt_exist{"doesnt_exist"};
// TODO ...:
// static std::string_view constexpr prev_fail{"prev_fail"};

} // namespace sort_by

// TODO ... EXPORT or HIDE?!
struct BUILD_CXX_DLL_EXPORT build_request_comparators_chain {
  // ret = -1 -> lhs < rhs; ret = 0 -> equal; ret = 1 -> rhs < lhs
  using comparator_fn = int(build_request const &lhs, build_request const &rhs,
                            common::fs_proxy *const fs);

  // not owning any pointer:
  using comparators_chain = std::vector<comparator_fn *>;

  explicit build_request_comparators_chain(
      common::fs_proxy *const aFs, comparators_chain const &aComps) noexcept;

  // true if `lhs` has higher priority (should be processed sooner) than `rhs`
  [[nodiscard]] bool operator()(build_request const &lhs,
                                build_request const &rhs) const;

  [[nodiscard]] static comparators_chain
  make_comparators_chain(std::vector<std::string_view> const &comparator_names);

private:
  common::fs_proxy *fs;
  comparator_fn *const *comps;
  std::size_t n_comps;
};

} // namespace build_cxx::driver
