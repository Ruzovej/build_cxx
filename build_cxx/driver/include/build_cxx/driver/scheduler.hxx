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
#include <build_cxx/common/macros.h>

namespace build_cxx::driver {

// This class is expected to be held & used from one particular thread. Or to be
// more precise: its public methods aren't thread-safe.
struct BUILD_CXX_DLL_EXPORT scheduler { // TODO should be BUILD_CXX_DLL_HIDE,
                                        // but then unit tests won't compile ...
  explicit scheduler(int const aN_workers) noexcept;

  ~scheduler() noexcept;

  // TODO rework this weird "private-public dance":
private:
  struct build_request {
    common::abstract_target *tgt{nullptr};
    std::vector<common::abstract_target const *> const *deps{nullptr};
  };

public:
  [[nodiscard]] int num_workers() const {
    // force 2 lines
    return n_workers;
  }

  void schedule_build(build_request task);

  [[nodiscard]] long long num_handled_targets() const {
    return n_handled_targets;
  }

  [[nodiscard]] common::abstract_target const *
  get_built_target(bool const blocking = true);

private:
  int n_workers;
  long long n_handled_targets{0};
  bool running{true}; // used only for destruction
  std::vector<std::thread> workers;

  std::mutex mtx_todo;
  std::condition_variable cv_todo;
  // TODO later change this to priority_queue (with customizable ordering, etc.)
  std::queue<build_request> todo;

  std::mutex mtx_done;
  std::condition_variable cv_done;
  std::queue<common::abstract_target const *> done;

  void spawn_worker_threads();
  void stop_worker_threads();

private:
  scheduler(const scheduler &) = delete;
  scheduler &operator=(const scheduler &) = delete;
  scheduler(scheduler &&) = delete;
  scheduler &operator=(scheduler &&) = delete;
};

} // namespace build_cxx::driver
