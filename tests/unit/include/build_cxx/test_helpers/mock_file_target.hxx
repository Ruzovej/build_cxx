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

#include "build_cxx/common/file_target.hxx"
#include "build_cxx/test_helpers/built_targets_t.hxx"

namespace build_cxx::test_helpers {

struct mock_file_target : common::file_target {
  using file_target::file_target;

  built_targets_t *built_targets{nullptr};

  void set_read_only(bool const aRead_only) {
    // force 2 lines
    simulated_read_only = aRead_only;
  }

  void set_fs_proxy(common::fs_proxy *aFs) {
    // force 2 lines
    fs = aFs;
  }

  void recipe(std::vector<common::abstract_target const *> const &resolved_deps)
      override {
    static_cast<void>(resolved_deps);

    if (!simulated_read_only) {
      if (built_targets) {
        built_targets->emplace(this);
      }
      fs->touch(resolved_path);
    }
  }

  void update_status(common::target_status const newest_dep_status) override {
    if (simulated_read_only) {
      status.merge_with(newest_dep_status);
    } else {
      file_target::update_status(newest_dep_status);
    }
  }

protected:
  bool simulated_read_only{false};
};

} // namespace build_cxx::test_helpers
