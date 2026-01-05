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

#include <limits>
#include <string_view>

#include "build_cxx/common/location.hxx"

namespace build_cxx::common {

struct abstract_target {
  explicit abstract_target(location const *const aLoc,
                           std::string_view const aName,
                           std::string_view const *const aDeps,
                           std::size_t const aNum_deps);

  virtual ~abstract_target() = default;

  // TODO
  // - use: https://en.cppreference.com/w/cpp/filesystem/file_time_type.html
  // - because of:
  // https://en.cppreference.com/w/cpp/filesystem/last_write_time.html
  // - comp. op.:
  // https://en.cppreference.com/w/cpp/chrono/time_point/operator_cmp.html
  // - example: https://godbolt.org/z/Enoza77Wo
  using modification_time_t = long long;
  static modification_time_t constexpr always_up_to_date{
      std::numeric_limits<long long>::min()};
  static modification_time_t constexpr never_up_to_date{
      std::numeric_limits<long long>::max()};
  [[nodiscard]] virtual modification_time_t last_modification_time() const = 0;

  // TODO private & getters, (setters?!), etc.:
  // "private":
  abstract_target *next{nullptr}; // non owned

  // "public":
  location const *loc; // non owned
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
