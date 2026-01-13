/*
  Copyright 2025 Lukáš Růžička

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

#include <filesystem>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <utility>

#include <build_cxx/common/file_target.hxx>
#include <build_cxx/common/phony_target.hxx>

#include "build_cxx/driver/dlopen_scoped.hxx"

namespace build_cxx::driver {

namespace {

std::pair<std::string, std::string>
normalize_file_paths(std::string_view const proj_root_file_path,
                     std::string_view const target_definition_file_path,
                     std::string_view const target_name) {
  std::pair<std::string, std::string> res{};

  if (target_name.empty()) {
    throw "Internal error: target_name must not be empty for normalization";
  } else if (target_name.front() == '/') {
    // absolute path - leave as is:
    res.first = "/";
    res.second = std::string{target_name.cbegin() + 1, target_name.cend()};
  } else {
    std::filesystem::path rfp{proj_root_file_path};
    rfp.remove_filename();

    std::filesystem::path tdfp{target_definition_file_path};
    tdfp.remove_filename();
    tdfp /= target_name;

    res.first = rfp.string();
    res.first.erase(res.first.size() - 1, 1); // remove trailing separator

    res.second =
        tdfp.string().erase(0, res.first.size() + 1); // +1 for separator
  }

  return res;
}

} // namespace

void processed_targets::process_project(common::project const *const proj) {
  // TODO checks that it's not already processed, etc.

  projects_by_name.emplace(proj->name, proj);

  for (auto at{proj->first}; at != nullptr; at = at->next) {
    targets_by_project[proj].emplace_back(at);

    project_of_target.emplace(at, proj);

    auto const normalized{
        normalize_file_paths(proj->root_file, at->loc->filename, at->name)};

    if (normalized.first != "/") {
      targets_by_path.emplace(target_path{proj->name, "", normalized.second},
                              at);
    }
    targets_by_path.emplace(
        target_path{"", normalized.first, normalized.second}, at);

    target_resolved_deps.emplace(at, resolved_deps{});

    ++unresolved;
  }
}

bool processed_targets::resolve_deps(common::abstract_target const *const at) {
  if (at == nullptr) {
    if (unresolved != 0) {
      for (auto &[key, val] : target_resolved_deps) {
        if (!val.resolved) {
          // internaly decrements by one `unresolved` in case of "success"
          val.resolved = resolve_deps(key);
        }
      }
    }
    return unresolved == 0;
  }

  auto const iter{target_resolved_deps.find(at)};
  if (iter == target_resolved_deps.cend()) {
    // TODO better type
    throw "Internal error: trying to resolve deps for unknown target";
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
    // target_path const at_tp{at_proj_name, at_fn, at->name};

    for (std::size_t idx{0}; idx < at->num_deps; ++idx) {
      std::string const dep{at->raw_deps[idx]};

      auto const try_resolve_dep = [&](target_path const &candidate_tp) {
        auto const iter{targets_by_path.find(candidate_tp)};

        if (iter != targets_by_path.cend()) {
          deps.emplace_back(iter->second);
          return true;
        }
        return false;
      };

      auto const normalized{normalize_file_paths(
          project_of_target.at(at)->root_file, at->loc->filename, dep)};

      bool const os_path{normalized.first == "/"};

      if (!try_resolve_dep(
              target_path{"", normalized.first, normalized.second}) &&
          !os_path &&
          !try_resolve_dep(target_path{at_proj_name, "", normalized.second})) {
        all_deps_resolved = false;
      } else {
        // clang-format off
        // TODO fix it ...
        // e.g. 
        // $ build/build_cxx/driver/build_cxx_driver build/tests/integration/lib02.so build/tests/integration/lib02.so build/tests/integration/lib02.so
        // works, but                                                            ^
        // $ build/build_cxx/driver/build_cxx_driver build/tests/integration/lib01.so build/tests/integration/lib02.so
        // doesn't ...                                                           ^ there's the crucial difference     ^^^... unimportant
        // clang-format on
        [[maybe_unused]] volatile bool LR_DBG1 = true;
        [[maybe_unused]] volatile bool LR_DBG2 = true;
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

} // namespace build_cxx::driver
