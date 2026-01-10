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

#include "build_cxx/common/abstract_target.hxx"

namespace build_cxx::common {

struct phony_target : abstract_target {
  explicit phony_target(location const *const loc, std::string_view const name,
                        std::string_view const *const raw_deps,
                        std::size_t const num_deps);

  virtual ~phony_target() = default;

  // Phony target is always out of date
  [[nodiscard]] modification_time_t last_modification_time() const override {
    return never_up_to_date;
  }

private:
  phony_target(phony_target const &) = delete;
  phony_target &operator=(phony_target const &) = delete;
  phony_target(phony_target &&) = delete;
  phony_target &operator=(phony_target &&) = delete;
};

} // namespace build_cxx::common
