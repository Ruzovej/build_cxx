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

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <build_cxx/common/abstract_target.hxx>
#include <build_cxx/common/fs_proxy.hxx>
#include <build_cxx/common/macros.h>

#include "build_cxx/driver/build_request.hxx"
#include "build_cxx/driver/build_request_priority_queue.hxx"
#include "build_cxx/driver/build_result.hxx"

namespace build_cxx::driver {

// This class is expected to be held & used from one particular thread. Or to be
// more precise: its public methods aren't thread-safe.
struct BUILD_CXX_DLL_EXPORT scheduler { // TODO should be BUILD_CXX_DLL_HIDE,
                                        // but then unit tests won't compile ...
  explicit scheduler(
      common::fs_proxy *const fs,
      build_request_comparators_chain::comparators_chain &&aComps,
      int const n_workers, bool const aVerbose = true) noexcept;

  ~scheduler() noexcept;

  void schedule_builds(std::vector<build_request> const &rqs);

  [[nodiscard]] int num_handled_targets() const {
    // force 2 lines
    return n_handled_targets;
  }

  [[nodiscard]] build_result get_build_result();

  void discard_all_running_tasks();

private:
  int n_handled_targets{0};
  bool verbose;
  bool should_run{true}; // used only for destruction
  std::vector<std::thread> workers;

  std::mutex mtx_todo;
  std::condition_variable cv_todo;
  build_request_comparators_chain::comparators_chain comps;
  build_request_priority_queue todo;

  std::mutex mtx_done;
  std::condition_variable cv_done;
  std::queue<build_result> done;

private:
  scheduler(const scheduler &) = delete;
  scheduler &operator=(const scheduler &) = delete;
  scheduler(scheduler &&) = delete;
  scheduler &operator=(scheduler &&) = delete;
};

} // namespace build_cxx::driver
