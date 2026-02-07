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

#include <memory>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <build_cxx/common/abstract_target.hxx>
#include <build_cxx/common/macros.h>
#include <build_cxx/common/project.hxx>

#include "build_cxx/driver/scheduler.hxx"

namespace build_cxx::driver {

// TODO should be BUILD_CXX_DLL_HIDE, but then unit tests won't compile ...
struct BUILD_CXX_DLL_EXPORT processed_targets {
  // "loader" should ensure that projects are unique, and if same project is
  // required/loaded twice, it has exactly same version
  explicit processed_targets(scheduler *const aSched) : sched{aSched} {
    // force 2 lines
  }

  // TODO hide as many as possible behind getters, setters, etc.:

  void process_project(common::project const *const proj);

  // not owning any pointer(s):
  std::unordered_map<std::string_view, common::project const *>
      projects_by_name;

  // not owning any pointer(s):
  std::unordered_map<common::project const *,
                     std::vector<common::abstract_target *>>
      targets_by_project;

  // not owning any pointer(s):
  std::unordered_map<common::abstract_target const *, common::project const *>
      project_of_target;

  // not owning any pointer(s):
  std::unordered_map<std::string_view, common::abstract_target *>
      targets_by_resolved_name;

  // TODO use later, or delete ...
  // (values) obviously owned here; targets implied by user-defined ones (e.g.
  // translation unit: `foo.cxx` -> `foo.o` may have intermediate target
  // `foo.cxx.pp` for preprocessed source):
  std::unordered_map<common::abstract_target const *,
                     std::vector<std::unique_ptr<common::abstract_target>>>
      intermediate_targets;

  // resolve as many deps as possible for either provided `at` or for all
  // "known" by default;
  // returned value indicates whether all is resolved
  [[nodiscard]] bool
  resolve_deps(common::abstract_target const *const at = nullptr);

  // TODO remove later ...
  [[nodiscard]] auto const &get_target_resolved_deps() const {
    return target_resolved_deps;
  }

  // TODO - methods for detecting cycles, etc.:
  // [[nodiscard]] bool
  // build_tree_valid_for(const common::abstract_target *const root) const;
  //
  // [[nodiscard]] bool whole_build_tree_valid() const;

  // TODO
  // - parallelize
  // - accept more targets at once: via `vector`, or even better -
  //   `unordered_set`
  // - switch for turning on/off logging to `stdout`, etc.
  void build_target(common::abstract_target *const tgt, bool const verbose);

  void build_all_targets(bool const verbose);

private:
  // TODO `unordered_set` instead of `vector` ...:
  void build_targets_impl(std::vector<common::abstract_target const *> &tgts,
                          std::string &indent, bool const verbose);

  // not owning any pointer(s):
  std::unordered_set<common::abstract_target const *> built_targets;

  struct resolved_deps {
    bool resolved{false};
    long long already_built{0};
    // not owning any pointer(s):
    std::vector<common::abstract_target const *> deps;
    std::vector<common::abstract_target const *> dep_of;
  };

  // not owning any pointer(s):
  std::unordered_map<common::abstract_target const *, resolved_deps>
      target_resolved_deps;

  long long unresolved{0};

  scheduler *sched{nullptr};

private:
  processed_targets(processed_targets const &) = delete;
  processed_targets &operator=(processed_targets const &) = delete;
  processed_targets(processed_targets &&) = delete;
  processed_targets &operator=(processed_targets &&) = delete;
};

} // namespace build_cxx::driver
