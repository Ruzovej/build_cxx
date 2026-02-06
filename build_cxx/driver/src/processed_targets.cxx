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

#include "build_cxx/driver/processed_targets.hxx"

#include <utility>

#include <build_cxx/common/file_target.hxx>
#include <build_cxx/common/phony_target.hxx>

#include "build_cxx/driver/dlopen_scoped.hxx"
#include "build_cxx/driver/scheduler.hxx"

namespace build_cxx::driver {

void processed_targets::process_project(common::project const *const proj) {
  // TODO checks that it's not already processed, etc.

  projects_by_name.emplace(proj->name, proj);

  for (auto at{proj->first}; at != nullptr; at = at->next) {
    at->resolve_own_traits();

    targets_by_project[proj].emplace_back(at);

    project_of_target.emplace(at, proj);

    targets_by_resolved_name.emplace(at->resolved_name, at);

    target_resolved_deps.emplace(at, resolved_deps{});

    ++unresolved;
  }
}

// TODO split into multiple methods:
bool processed_targets::resolve_deps(common::abstract_target const *const at) {
  if (at == nullptr) {
    if (unresolved != 0) {
      for (auto &[key, val] : target_resolved_deps) {
        if (!val.resolved) {
          // internally decrements by one `unresolved` in case of "success"
          val.resolved = resolve_deps(key);
        }
      }
    }
    return unresolved == 0;
  }

  auto const iter{target_resolved_deps.find(at)};
  if (iter == target_resolved_deps.cend()) {
    throw std::runtime_error{
        "Internal error: trying to resolve deps for unknown target " +
        std::string{at->name}};
  }

  if (iter->second.resolved) {
    return true;
  }

  bool all_deps_resolved{true};
  std::vector<common::abstract_target const *> deps;

  { // the heavy lifting ...:
    deps.reserve(at->num_deps);

    auto const at_proj_name{project_of_target.at(at)->name};

    auto const at_parent_path{
        std::filesystem::path{at->loc->filename}.parent_path().string()};

    for (std::size_t idx{0}; idx < at->num_deps; ++idx) {
      std::string_view const dep{at->raw_deps[idx]};

      // TODO simplify
      auto const try_resolve_dep =
          [&](std::string_view const candidate_resolved_name) {
            auto const iter{
                targets_by_resolved_name.find(candidate_resolved_name)};

            if (iter != targets_by_resolved_name.cend()) {
              deps.emplace_back(iter->second);
              target_resolved_deps[iter->second].dep_of.emplace_back(at);
              return true;
            }
            return false;
          };

      // TODO simplify
      if (try_resolve_dep(dep)) {
        // unchanged name -> phony target from other project
      } else if (try_resolve_dep(
                     common::phony_target::resolve_name(at_proj_name, dep))) {
        // phony target from this project
      } else if (dep.at(0) == '/') {
        // absolute path - don't alter it
      } else if (try_resolve_dep(
                     common::file_target::resolve_path(at->loc->filename, dep)
                         .string())) {
        // file target from this project - resolved path
      } else {
        all_deps_resolved = false;
      }
    }
  }

  if (all_deps_resolved) {
    --unresolved;
  }

  iter->second.deps = std::move(deps);

  return all_deps_resolved;
}

void processed_targets::build_target(common::abstract_target *const tgt,
                                     bool const verbose) {
  std::vector<common::abstract_target const *> tgts{tgt};
  std::string indent{};
  build_targets_impl(tgts, indent, verbose);
}

void processed_targets::build_all_targets(bool const verbose) {
  for (auto const [_, tgts] : targets_by_project) {
    for (auto const tgt : tgts) {
      build_target(tgt, verbose);
    }
  }
}

// TODO split into multiple methods:
void processed_targets::build_targets_impl(
    std::vector<common::abstract_target const *> &tgts, std::string &indent,
    bool const verbose) {
  std::unordered_set<common::abstract_target const *> unsatisfied_deps;
  std::unordered_set<common::abstract_target const *> satisfied_deps;

  while (!tgts.empty()) {
    auto *const tgt{tgts.back()};
    tgts.pop_back();

    if (built_targets.count(tgt) == 1) {
      continue;
    }

    auto const &resolved_deps{target_resolved_deps.at(tgt)};

    if (resolved_deps.already_built == resolved_deps.deps.size()) {
      satisfied_deps.emplace(tgt);
    } else {
      unsatisfied_deps.emplace(tgt);
    }

    for (auto const dep : resolved_deps.deps) {
      if ((unsatisfied_deps.count(dep) == 0) &&
          (satisfied_deps.count(dep) == 0)) {
        tgts.emplace_back(dep);
      }
    }
  }

  // TODO number of jobs from parameter
  scheduler sched{12};

  do {
    while (!satisfied_deps.empty()) {
      auto it{satisfied_deps.begin()};
      auto *const tgt{*it};
      satisfied_deps.erase(it);

      auto const &tgt_resolved_deps{target_resolved_deps.at(tgt)};

      if constexpr (false) {
        // TODO get rid of this `const_cast` ...:
        sched.schedule_build({const_cast<common::abstract_target *>(tgt),
                              &tgt_resolved_deps.deps});
      } else {
        // but this way?!
        sched.schedule_build(
            {const_cast<common::abstract_target *>(
                 targets_by_resolved_name.at(tgt->resolved_name)),
             &tgt_resolved_deps.deps});
      }
    }

    auto const *const built_tgt{sched.get_built_target(true)};

    // TODO rework this so this nullptr check isn't needed here:
    if (built_tgt == nullptr) {
      continue;
    }

    built_targets.emplace(built_tgt);

    auto const &built_tgt_resolved_deps{target_resolved_deps.at(built_tgt)};

    for (auto *const consumer : built_tgt_resolved_deps.dep_of) {
      auto &consumer_resolved_deps{target_resolved_deps.at(consumer)};

      ++consumer_resolved_deps.already_built;

      auto const iter{unsatisfied_deps.find(consumer)};

      if (iter == unsatisfied_deps.cend()) {
        continue;
      }

      if (consumer_resolved_deps.already_built ==
          consumer_resolved_deps.deps.size()) {
        unsatisfied_deps.erase(iter);
        satisfied_deps.emplace(consumer);
      }
    }
  } while ((sched.num_handled_targets() != 0) || (!satisfied_deps.empty()));

  if (!unsatisfied_deps.empty()) {
    throw std::runtime_error{"Build order of targets contains a cycle"};
  }
}

} // namespace build_cxx::driver
