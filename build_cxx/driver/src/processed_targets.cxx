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

#include "build_cxx/driver/scheduler.hxx"

namespace build_cxx::driver {

processed_targets::processed_targets(scheduler &aSched) noexcept
    : sched{aSched} {
  // force 2 lines
}

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

bool processed_targets::resolve_deps_for_all() {
  if (unresolved != 0) {
    for (auto &[key, val] : target_resolved_deps) {
      if (!val.resolved) {
        // decrements `unresolved` in case of "success":
        static_cast<void>(resolve_deps_for(key));
      }
    }
  }
  return unresolved == 0;
}

// TODO split into multiple methods:
bool processed_targets::resolve_deps_for(
    common::abstract_target const *const at) {
  if (at == nullptr) {
    throw std::runtime_error{
        "Internal error: no target provided to resolve_deps_for()"};
  }

  auto const iter{target_resolved_deps.find(at)};
  if (iter == target_resolved_deps.cend()) {
    throw std::runtime_error{
        "Internal error: trying to resolve deps for unknown target " +
        std::string{at->name}};
  }

  auto &res_deps{iter->second};

  if (res_deps.resolved) {
    return true;
  }

  bool all_deps_resolved{true};
  std::vector<common::abstract_target const *> deps;
  deps.reserve(at->num_deps);

  // TODO separate method
  auto const try_resolve_dep =
      [&](std::string_view const candidate_resolved_name) {
        auto const iter{targets_by_resolved_name.find(candidate_resolved_name)};

        if (iter != targets_by_resolved_name.cend()) {
          deps.emplace_back(iter->second);
          target_resolved_deps.at(iter->second).dep_of.emplace(at);
          return true;
        }
        return false;
      };

  auto const at_proj_name{project_of_target.at(at)->name};

  for (std::size_t idx{0}; all_deps_resolved && (idx < at->num_deps); ++idx) {
    std::string_view const dep{at->raw_deps[idx]};

    // TODO simplify it even more
    // using short-circuiting with `OR` to avoid unnecessary checks:
    all_deps_resolved =
        // unchanged name -> (probably) phony target from other project:
        try_resolve_dep(dep)
        // phony target from this project:
        ||
        try_resolve_dep(common::phony_target::resolve_name(at_proj_name, dep))
        // absolute path - don't alter it:
        || (dep.at(0) == '/') // TODO is it OK not to update `deps` and
                              // `target_resolved_deps[...].dep_of`?!
        // file target from this project - resolved path
        ||
        try_resolve_dep(
            common::file_target::resolve_path(at->loc->filename, dep).string());
  }

  if (all_deps_resolved) {
    --unresolved;

    res_deps.resolved = true;
    res_deps.deps = std::move(deps);
  }

  return all_deps_resolved;
}

void processed_targets::build_target(std::string_view const tgt,
                                     bool const verbose) {
  std::vector<std::string_view> tgts{tgt};

  build_targets(tgts, verbose);
}

void processed_targets::build_target(common::abstract_target const *const tgt,
                                     [[maybe_unused]] bool const verbose) {
  std::vector<common::abstract_target const *> tgts{tgt};

  build_targets(tgts, verbose);
}

void processed_targets::build_targets(std::vector<std::string_view> const &tgts,
                                      bool const verbose) {
  std::vector<common::abstract_target const *> resolved_tgts{};
  resolved_tgts.reserve(tgts.size());

  for (auto const &tgt : tgts) {
    auto const iter{targets_by_resolved_name.find(tgt)};

    if (iter == targets_by_resolved_name.cend()) {
      throw std::runtime_error{"Requested target '" + std::string{tgt} +
                               "' not found"};
    }

    resolved_tgts.emplace_back(iter->second);
  }

  build_targets(resolved_tgts, verbose);
}

void processed_targets::build_targets(
    std::vector<common::abstract_target const *> &tgts,
    [[maybe_unused]] bool const verbose) {
  build_targets_impl(tgts);
}

void processed_targets::build_all_targets([[maybe_unused]] bool const verbose) {
  std::vector<common::abstract_target const *> all_tgts{};

  for (auto const &[_, tgts] : targets_by_project) {
    for (auto *const tgt : tgts) {
      if (tgt->include_in_all) {
        all_tgts.emplace_back(tgt);
      }
    }
  }

  build_targets_impl(all_tgts);
}

// TODO split into multiple methods:
void processed_targets::build_targets_impl(
    std::vector<common::abstract_target const *> &tgts) {
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

    for (auto *const dep : resolved_deps.deps) {
      if ((unsatisfied_deps.count(dep) == 0) &&
          (satisfied_deps.count(dep) == 0)) {
        tgts.emplace_back(dep);
      }
    }
  }

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
        auto *mtgt{targets_by_resolved_name.at(tgt->resolved_name)};
        sched.schedule_build({mtgt, &tgt_resolved_deps.deps});
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
