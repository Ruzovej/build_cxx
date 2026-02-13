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

//#include <iostream>
#include <mutex>
//#include <thread>

#include "build_cxx/common/phony_target.hxx"
#include "build_cxx/test_helpers/built_targets_t.hxx"

namespace build_cxx::test_helpers {

struct mock_phony_target : common::phony_target {
  using phony_target::phony_target;

  std::mutex *mtx{nullptr};
  built_targets_t *built_targets{nullptr};

  void recipe(std::vector<common::abstract_target const *> const &resolved_deps)
      const override {
    static_cast<void>(resolved_deps);

    //{
    //  std::lock_guard lck{*mtx};
    //  std::cout << std::this_thread::get_id() << ": start building "
    //            << resolved_name << std::endl;
    //}

    if (built_targets) {
      std::lock_guard lck{*mtx};
      built_targets->emplace(this);
    }

    //{
    //  std::lock_guard lck{*mtx};
    //  std::cout << std::this_thread::get_id() << ": finished building "
    //            << resolved_name << std::endl;
    //}
  }
};

} // namespace build_cxx::test_helpers
