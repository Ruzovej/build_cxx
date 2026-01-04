/*
  Copyright 2025 Lukáš Růžička

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

#include <string_view>

#include "build_cxx/common/location.hxx"

namespace build_cxx::common {

struct abstract_target {
  explicit abstract_target(location const *const aLoc,
                           std::string_view const aName,
                           std::string_view const *const aDeps,
                           std::size_t const aNum_deps)
      : loc{aLoc}, name{aName}, deps{aDeps}, num_deps{aNum_deps} {}

  virtual ~abstract_target() = default;

  // [[nodiscard]] virtual long long last_modification_time() const = 0;
  //
  // [[nodiscard]] virtual bool is_up_to_date() const;
  //
  // virtual void update() = 0;

  // non owned:
  abstract_target *next{nullptr};
  location const *loc;

  // TODO private & getters, (setters?!), etc.:
  // std::string_view target_namespace;
  std::string_view name;
  std::string_view const *deps;
  std::size_t num_deps;

private:
  abstract_target(abstract_target const &) = delete;
  abstract_target &operator=(abstract_target const &) = delete;
  abstract_target(abstract_target &&) = delete;
  abstract_target &operator=(abstract_target &&) = delete;
};

} // namespace build_cxx::common
