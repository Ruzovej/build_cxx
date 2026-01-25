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
#include "build_cxx/test_helpers/built_targets_t.hxx"

namespace build_cxx::test_helpers {

struct mock_file_target : common::file_target {
  using file_target::file_target;

  built_targets_t *built_targets{nullptr};

  void set_mod_times(
      std::optional<common::target_status::file_modification_time_t> const
          init_mod_time,
      std::optional<common::target_status::file_modification_time_t> const
          final_mod_time = std::nullopt,
      bool const reinitialize_status = true) {
    simulated_mod_time = init_mod_time;

    if (reinitialize_status) {
      initialize_status(); // so `status` gets updated with this new value ...
    }

    simulated_mod_time_after_update = final_mod_time;
  }

  void rm() {
    simulated_mod_time.reset();
    initialize_status(); // so `status` gets updated with this new value ...
  }

  void set_read_only(bool const aRead_only) {
    simulated_read_only = aRead_only;
  }

  void recipe(std::vector<common::abstract_target const *> const &resolved_deps)
      override {
    if (!simulated_read_only) {
      if (built_targets) {
        built_targets->emplace(this);
      }
    }
  }

  void update_status() override {
    if (!simulated_read_only) {
      simulated_mod_time = simulated_mod_time_after_update;
      initialize_status(); // so `status` gets updated with this new value ...
    }
  }

protected:
  std::optional<common::target_status::file_modification_time_t>
      simulated_mod_time;

  std::optional<common::target_status::file_modification_time_t>
      simulated_mod_time_after_update;

  bool simulated_read_only{false};

  [[nodiscard]] common::target_status compute_status() const override {
    return common::target_status{simulated_mod_time.value()};
  }

  [[nodiscard]] bool exists() const override {
    return simulated_mod_time.has_value();
  }
};

} // namespace build_cxx::test_helpers
