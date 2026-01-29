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

  void set_read_only(bool const aRead_only) {
    simulated_read_only = aRead_only;
  }

  void set_fs_proxy(common::fs_proxy *aFs) { fs = aFs; }

  void recipe(std::vector<common::abstract_target const *> const
                  & /*resolved_deps*/) override {
    if (!simulated_read_only) {
      if (built_targets) {
        built_targets->emplace(this);
      }
      fs->touch(resolved_path);
    }
  }

  void update_status() override {
    if (!simulated_read_only) {
      fs->touch(resolved_path);
      initialize_status(); // so `status` gets updated with this new value ...
    }
  }

protected:
  bool simulated_read_only{false};

  //[[nodiscard]] common::target_status compute_status() const override {
  //  return common::target_status{simulated_mod_time.value()};
  //}

  [[nodiscard]] bool exists() const override {
    return fs->file_exists(resolved_path);
  }
};

} // namespace build_cxx::test_helpers
