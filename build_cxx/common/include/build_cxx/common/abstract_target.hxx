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

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "build_cxx/common/location.hxx"
#include "build_cxx/common/macros.h"

namespace build_cxx::common {

struct project;

struct BUILD_CXX_DLL_EXPORT abstract_target {
  explicit abstract_target(location const *const aLoc,
                           bool const aInclude_in_all,
                           std::string_view const aName,
                           std::string_view const *const aRaw_deps,
                           std::size_t const aNum_deps) noexcept;

  virtual ~abstract_target() = default;

  // TODO
  // - use: https://en.cppreference.com/w/cpp/filesystem/file_time_type.html
  // - because of:
  // https://en.cppreference.com/w/cpp/filesystem/last_write_time.html
  // - comp. op.:
  // https://en.cppreference.com/w/cpp/chrono/time_point/operator_cmp.html
  // - example: https://godbolt.org/z/Enoza77Wo
  using modification_time_t = long long;

  // nullopt means "always out of date"
  [[nodiscard]] virtual std::optional<modification_time_t>
  last_modification_time() const = 0;

  virtual void resolve_own_traits() = 0;

  virtual void
  recipe(std::vector<abstract_target const *> const &resolved_deps) = 0;

  virtual void
  build(std::vector<abstract_target const *> const &resolved_deps) = 0;

  // TODO private & getters, (setters?!), etc.:
  // "private":
  abstract_target *next{nullptr}; // non owned

  // "public":
  project const *parent_project{nullptr}; // non owned
  location const *loc;                    // non owned
  bool include_in_all;
  std::string_view name;
  std::string_view const *raw_deps; // non owned
  std::size_t num_deps;

  std::string_view resolved_kind;
  std::string resolved_name;

private:
  abstract_target(abstract_target const &) = delete;
  abstract_target &operator=(abstract_target const &) = delete;
  abstract_target(abstract_target &&) = delete;
  abstract_target &operator=(abstract_target &&) = delete;
};

} // namespace build_cxx::common
