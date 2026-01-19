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

#include <unordered_set>

#include "build_cxx/common/file_target.hxx"

namespace build_cxx::common {

struct test_file_target : file_target {
  using file_target::file_target;

  std::unordered_set<abstract_target const *> *built_targets{nullptr};

  void touch(modification_time_t const new_time) {
    simulated_mod_time = new_time;
  }

  void touch_inc(modification_time_t const inc_time = 1) {
    simulated_mod_time += inc_time;
  }

  void set_exists(bool const exists) { simulated_existence = exists; }

  [[nodiscard]] modification_time_t last_modification_time() const override {
    if (!simulated_existence) {
      return std::numeric_limits<modification_time_t>::min();
    }

    return simulated_mod_time;
  }

  void build(std::vector<abstract_target const *> const & /*deps*/) override {
    if (built_targets) {
      built_targets->emplace(this);
    }
  }

protected:
  modification_time_t simulated_mod_time{0};
  bool simulated_existence{true};
};

} // namespace build_cxx::common
