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

#include "build_cxx/driver/scheduler.hxx"

#include <algorithm>
#include <stdexcept>

namespace build_cxx::driver {

scheduler::scheduler(int const aN_workers) noexcept
    : n_workers{std::max(aN_workers, 1)} {
  spawn_worker_threads();
}

scheduler::~scheduler() noexcept {
  // force 2 lines
  stop_worker_threads();
}

void scheduler::schedule_build(scheduler::build_request task) {
  ++n_handled_targets;
  {
    std::lock_guard lck{mtx_todo};
    todo.push(task);
  }
  cv_todo.notify_one();
}

common::abstract_target const *
scheduler::get_built_target(bool const blocking) {
  if (n_handled_targets == 0) {
    // TODO maybe rather throw exception, so user learns not to call it in this
    // case - which can be checked, etc.:
    return nullptr;
  }

  std::unique_lock lck{mtx_done};

  if (blocking && done.empty()) {
    cv_done.wait(lck, [this]() {
      // force 2 lines
      return !done.empty();
    });
  }

  if (!done.empty()) {
    auto *const res{done.front()};
    done.pop();
    --n_handled_targets;
    return res;
  }

  return nullptr;
}

void scheduler::spawn_worker_threads() {
  workers.reserve(n_workers);

  for (int idx{0}; idx < n_workers; ++idx) {
    workers.emplace_back([this]() {
      while (true) {
        build_request task;
        {
          std::unique_lock lck{mtx_todo};
          cv_todo.wait(lck, [&]() {
            // force 2 lines
            return !running || !todo.empty();
          });

          if (!running) {
            return;
          }

          task = todo.front();
          todo.pop();
        }

        task.tgt->build(*task.deps);

        {
          std::lock_guard lck{mtx_done};
          done.push(task.tgt);
        }
        cv_done.notify_one();
      }
    });
  }
}

void scheduler::stop_worker_threads() {
  {
    std::lock_guard lck{mtx_todo};
    running = false;
  }
  cv_todo.notify_all();

  for (auto &worker : workers) {
    worker.join();
  }
}

} // namespace build_cxx::driver
