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

#include <iostream>
#include <stdexcept>

namespace build_cxx::driver {

scheduler::scheduler(int const n_workers, bool const aVerbose) noexcept
    : verbose{aVerbose} {
  workers.reserve(std::max(n_workers, 1));

  for (int idx{0}; idx < workers.capacity(); ++idx) {
    workers.emplace_back([this]() {
      while (true) {
        build_result res;

        try {
          build_request task;

          {
            std::unique_lock lck{mtx_todo};
            cv_todo.wait(lck, [&]() {
              // force 2 lines
              return !should_run || !todo.empty();
            });

            if (!should_run) {
              break;
            }

            task = todo.front();
            todo.pop();
          }

          res.tgt = task.tgt;

          task.tgt->build(*task.deps);

          res.success = true;
        } catch (std::exception const &e) {
          if (verbose) {
            if (res.tgt) {
              std::cerr << "Error in worker thread when processing "
                        << res.tgt->name << "': " << e.what() << '\n';
            } else {
              std::cerr << "Error in worker thread: " << e.what() << '\n';
            }
          }
        } catch (...) {
          if (verbose) {
            if (res.tgt) {
              std::cerr << "Unknown error in worker thread when processing "
                        << res.tgt->name << "'\n";
            } else {
              std::cerr << "Unknown error in worker thread\n";
            }
          }
        }

        {
          std::lock_guard lck{mtx_done};
          done.push(res);
        }

        cv_done.notify_one();
      }
    });
  }
}

scheduler::~scheduler() noexcept {
  {
    std::lock_guard lck{mtx_todo};
    should_run = false;
  }

  cv_todo.notify_all();

  for (auto &worker : workers) {
    worker.join();
  }
}

void scheduler::schedule_build(
    common::abstract_target *const tgt,
    std::vector<common::abstract_target const *> const *const deps) {
  ++n_handled_targets;

  {
    std::lock_guard lck{mtx_todo};
    todo.push({tgt, deps});
  }

  cv_todo.notify_one();
}

common::abstract_target const *scheduler::get_built_target() {
  if (n_handled_targets == 0) {
    throw std::runtime_error{
        "Workers are idle, without any job to complete nor to hand over"};
  }

  build_result res;

  {
    std::unique_lock lck{mtx_done};

    cv_done.wait(lck, [this]() {
      // force 2 lines
      return !done.empty();
    });

    res = done.front();
    done.pop();
  }

  --n_handled_targets;

  if (res.tgt == nullptr) {
    throw std::runtime_error{
        "Serious error - worker failed to provide (finished) target"};
  } else if (!res.success) {
    throw std::runtime_error{"Failed to build target '" +
                             res.tgt->resolved_name + '\''};
  }

  return res.tgt;
}

} // namespace build_cxx::driver
