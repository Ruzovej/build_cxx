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

#include <string>
#include <string_view>
#include <vector>

#include "build_cxx/common/location.hxx"
#include "build_cxx/common/macros.h"
#include "build_cxx/common/target_status.hxx"

namespace build_cxx::common {

struct project;

struct BUILD_CXX_DLL_EXPORT abstract_target {
  explicit abstract_target(location const *const aLoc,
                           bool const aInclude_in_all,
                           std::string_view const aName,
                           std::string_view const *const aRaw_deps,
                           std::size_t const aNum_deps) noexcept;

  virtual ~abstract_target() = default;

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // "private":
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // for initialization
  virtual void resolve_own_traits() = 0;

  // for building

  virtual void initialize_status() = 0;
  virtual void update_status(target_status const newest_dep_status) = 0;

  [[nodiscard]] target_status get_status() const {
    // force 2 lines
    return status;
  }

  // don't call this in client code, only provide implementation; it can be
  // assumed:
  // - all `resolved_deps` are ... well, "resolved" :-)
  // - `recipe()` was already called for all `resolved_deps` (if needed)
  // - all `resolved_deps` have properly determined/updated `status`
  // it must be:
  // - called at most once for each target
  // - thread safe
  virtual void
  recipe(std::vector<abstract_target const *> const &resolved_deps) const = 0;

  // TODO getters, (setters?!), etc.:
  abstract_target *next{nullptr}; // non owned

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // "public":
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // TODO private & getters, (setters?!), etc.:
  project const *parent_project{nullptr}; // non owned
  location const *loc;                    // non owned
  bool include_in_all;
  std::string_view name;
  std::string_view const *raw_deps; // non owned
  std::size_t num_deps;

  std::string_view resolved_kind;
  std::string resolved_name;

protected:
  target_status status;

private:
  abstract_target(abstract_target const &) = delete;
  abstract_target &operator=(abstract_target const &) = delete;
  abstract_target(abstract_target &&) = delete;
  abstract_target &operator=(abstract_target &&) = delete;
};

} // namespace build_cxx::common
