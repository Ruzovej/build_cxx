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

#include <stdexcept>
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
    for (auto &[at, res_deps] : target_resolved_deps) {
      // decrements `unresolved` in case of "success":
      static_cast<void>(resolve_deps_for_impl(at, res_deps));
    }
  }
  return unresolved == 0;
}

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

  return resolve_deps_for_impl(at, iter->second);
}

void processed_targets::build_target(std::string_view const tgt,
                                     bool const verbose) {
  std::vector<std::string_view> tgts{tgt};

  build_targets(tgts, verbose);
}

void processed_targets::build_target(common::abstract_target const *const tgt,
                                     [[maybe_unused]] bool const verbose) {
  build_targets({tgt}, verbose);
}

void processed_targets::build_targets(std::vector<std::string_view> const &tgts,
                                      bool const verbose) {
  std::vector<common::abstract_target const *> resolved_tgts;
  resolved_tgts.reserve(tgts.size());

  for (auto const &tgt : tgts) {
    auto const iter{targets_by_resolved_name.find(tgt)};

    if (iter == targets_by_resolved_name.cend()) {
      throw std::runtime_error{"Requested target '" + std::string{tgt} +
                               "' not found"};
    }

    resolved_tgts.emplace_back(iter->second);
  }

  build_targets(std::move(resolved_tgts), verbose);
}

void processed_targets::build_targets(
    std::vector<common::abstract_target const *> &&tgts,
    [[maybe_unused]] bool const verbose) {
  build_targets_impl(std::move(tgts));
}

void processed_targets::build_all([[maybe_unused]] bool const verbose) {
  std::vector<common::abstract_target const *> all_tgts;

  for (auto const &[_, tgts] : targets_by_project) {
    for (auto *const tgt : tgts) {
      if (tgt->include_in_all) {
        all_tgts.emplace_back(tgt);
      }
    }
  }

  build_targets_impl(std::move(all_tgts));
}

common::abstract_target const *processed_targets::try_to_determine_target(
    std::string_view const dep_raw_name,
    std::string_view const relative_to_project,
    std::string_view const relative_to_file) const {
  if (auto *const dep{find_target_by_resolved_name(dep_raw_name)};
      dep != nullptr) {
    // unchanged name -> probably phony/alias target (from other project?), or
    // absolute path:
    return dep;
  }

  if (auto *const dep{
          find_target_by_resolved_name(common::phony_target::resolve_name(
              relative_to_project, dep_raw_name))};
      dep != nullptr) {
    // phony/alias target from this project:
    return dep;
  }

  if (auto *const dep{find_target_by_resolved_name(
          common::file_target::resolve_path(relative_to_file, dep_raw_name)
              .string())};
      dep != nullptr) {
    // file target from this project:
    return dep;
  }

  // not found ... abort the search:
  return nullptr;
}

common::abstract_target const *processed_targets::find_target_by_resolved_name(
    std::string_view const tgt_resolved_name) const {
  auto const iter{targets_by_resolved_name.find(tgt_resolved_name)};

  if (iter != targets_by_resolved_name.cend()) {
    return iter->second;
  }

  return nullptr;
}

processed_targets::categorized_targets
processed_targets::get_all_dependencies_of(
    std::vector<common::abstract_target const *> &&tgts) {
  categorized_targets res;

  while (!tgts.empty()) {
    auto *const tgt{tgts.back()};
    tgts.pop_back();

    if (built_targets.find(tgt) != built_targets.cend()) {
      continue;
    }

    auto const &resolved_deps{target_resolved_deps.at(tgt)};

    if (!resolved_deps.resolved) {
      throw std::runtime_error{"Trying to build target '" + tgt->resolved_name +
                               "' with unresolved dependencies"};
    }

    if (resolved_deps.already_built == resolved_deps.deps.size()) {
      res.buildable_targets.emplace(tgt);
    } else {
      res.blocked_targets.emplace(tgt);
    }

    for (auto *const dep : resolved_deps.deps) {
      if ((res.blocked_targets.find(dep) == res.blocked_targets.cend()) &&
          (res.buildable_targets.find(dep) == res.buildable_targets.cend())) {
        tgts.emplace_back(dep);
      }
    }
  }

  return res;
}

build_request processed_targets::make_build_request(
    common::abstract_target const *const tgt) {
  auto const &tgt_resolved_deps{target_resolved_deps.at(tgt)};

  if constexpr (false) {
    // TODO get rid of this `const_cast` ...:
    return {const_cast<common::abstract_target *>(tgt),
            &tgt_resolved_deps.deps};
  } else {
    // but this way?!
    auto *const mtgt{targets_by_resolved_name.at(tgt->resolved_name)};

    if (tgt != mtgt) {
      throw std::runtime_error{
          "Serious internal error - failure in 'const_cast' substitution"};
    }

    return {mtgt, &tgt_resolved_deps.deps};
  }
}

void processed_targets::process_target_build(
    common::abstract_target const *const tgt) {
  auto brq{make_build_request(tgt)};

  brq.tgt->initialize_status();

  auto worst_status{brq.tgt->get_worst_dep_status(*brq.deps)};

  if (brq.tgt->get_status().needs_update_compared_to(worst_status)) {
    brq.newest_dep_status = worst_status;
    pending_build_requests.emplace_back(std::move(brq));
  } else {
    up_to_date_targets.emplace_back(build_result{brq.tgt, worst_status, true});
  }
}

void processed_targets::process_target_builds(
    std::unordered_set<common::abstract_target const *> &tgts) {
  for (auto *const tgt : tgts) {
    process_target_build(tgt);
  }
  tgts.clear();
}

void processed_targets::scheduler_commit_build_requests() {
  sched.schedule_builds(pending_build_requests);
  pending_build_requests.clear();
}

bool processed_targets::any_pending_results() const {
  return !up_to_date_targets.empty() || (sched.num_handled_targets() != 0);
}

common::abstract_target const *processed_targets::get_built_target() {
  auto const any_trivial{!up_to_date_targets.empty()};

  auto const build_result{
      any_trivial ? up_to_date_targets.back()
                  : sched.get_build_result() // if not valid, exception has been
                                             // already thrown
  };

  if (any_trivial) {
    up_to_date_targets.pop_back();
  }

  build_result.tgt->update_status(build_result.newest_dep_status);

  built_targets.emplace(build_result.tgt);

  return build_result.tgt;
}

void processed_targets::build_targets_impl(
    std::vector<common::abstract_target const *> &&tgts) {
  if (tgts.empty()) {
    // TODO
    // - throw or log error/warning/info that nothing to build was provided?!
    // - or maybe ditch this check, and adjust the ones below, etc., to this
    // purpose
    return;
  }

  auto to_build{get_all_dependencies_of(std::move(tgts))};

  if (to_build.buildable_targets.empty() && to_build.blocked_targets.empty()) {
    // nothing to build - all has been built in prev rounds, etc.; this is
    // very unit-test specific situation - TODO rework it & don't invoke it
    // this way in unit tests?!
    return;
  }

  process_target_builds(to_build.buildable_targets);
  scheduler_commit_build_requests();

  do {
    auto *const built_tgt{get_built_target()};

    auto const &built_tgt_resolved_deps{target_resolved_deps.at(built_tgt)};

    for (auto *const consumer : built_tgt_resolved_deps.dep_of) {
      auto &consumer_resolved_deps{target_resolved_deps.at(consumer)};

      ++consumer_resolved_deps.already_built;

      auto const iter{to_build.blocked_targets.find(consumer)};

      if (iter == to_build.blocked_targets.cend()) {
        // `consumer` wasn't requested or isn't needed
        continue;
      }

      if (consumer_resolved_deps.already_built ==
          consumer_resolved_deps.deps.size()) {
        to_build.blocked_targets.erase(iter);
        process_target_build(consumer);
      }
    }
    scheduler_commit_build_requests();
  } while (any_pending_results());

  if (!to_build.blocked_targets.empty()) {
    throw std::runtime_error{"Build order of targets contains a cycle - no "
                             "target with satisfied deps is available"};
  }
}

bool processed_targets::resolve_deps_for_impl(
    common::abstract_target const *const at, resolved_deps &res_deps) {
  if (res_deps.resolved) {
    return true;
  }

  res_deps.deps.reserve(at->num_deps);

  auto const at_proj_name{project_of_target.at(at)->name};

  for (std::size_t idx{0}; idx < at->num_deps; ++idx) {
    auto *const depends_on_tgt{try_to_determine_target(
        at->raw_deps[idx], at_proj_name, at->loc->filename)};

    if (depends_on_tgt == nullptr) {
      // prevent having there duplicates if it succeeds some next time:
      res_deps.deps.clear();
      return false;
    }

    // properly link them together:
    res_deps.deps.emplace_back(depends_on_tgt);
    target_resolved_deps.at(depends_on_tgt).dep_of.emplace(at);
  }

  res_deps.resolved = true;
  --unresolved;

  return true;
}

} // namespace build_cxx::driver
