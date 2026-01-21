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

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <utility>

#include <build_cxx/common/file_target.hxx>
#include <build_cxx/common/phony_target.hxx>

#include "build_cxx/driver/dlopen_scoped.hxx"

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

  iter->second.resolved = all_deps_resolved;
  iter->second.deps = std::move(deps);

  return all_deps_resolved;
}

void processed_targets::build_target(common::abstract_target *const tgt,
                                     bool const verbose) {
  std::string indent{};
  build_target_impl(tgt, indent, verbose);
}

void processed_targets::build_all_targets(bool const verbose) {
  for (auto const [_, tgts] : targets_by_project) {
    for (auto const tgt : tgts) {
      build_target(tgt, verbose);
    }
  }
}

void processed_targets::build_target_impl(common::abstract_target *const tgt,
                                          std::string &indent,
                                          bool const verbose) {
  if (built_targets.find(tgt) != built_targets.cend()) {
    return;
  } else if (target_resolved_deps.find(tgt) == target_resolved_deps.cend()) {
    throw std::runtime_error{
        "Internal error: trying to build unknown target '" +
        std::string{tgt->name} + "'"};
  } else if (!target_resolved_deps.at(tgt).resolved) {
    throw std::runtime_error{"Internal error: trying to build target '" +
                             std::string{tgt->name} +
                             "' with unresolved dependencies"};
  }

  if (verbose) {
    std::cout << indent << "Building target '" << tgt->resolved_name << "':\n";
    indent += '\t';
  }

  // TODO remove later ...
  // if (tgt->name == "build/src/CCC.cxx.o") {
  //  [[maybe_unused]] volatile bool a = false;
  //  a = true;
  //}

  auto const &deps{target_resolved_deps.at(tgt).deps};

  for (auto const dep : deps) {
    // TODO get rid of this ugly `const_cast` ...
    build_target_impl(const_cast<common::abstract_target *>(dep), indent,
                      verbose);
  }

  auto const last_mod_time{tgt->last_modification_time()};

  if (verbose) {
    indent.pop_back();
    std::cout << indent << "-> ";
  }

  tgt->build(deps);
  
  if (verbose) {
    std::cout << '\n';
  }

  built_targets.emplace(tgt);
}

} // namespace build_cxx::driver
