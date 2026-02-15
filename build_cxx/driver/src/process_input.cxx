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

#include "build_cxx/driver/process_input.hxx"

#include <functional>
#include <iostream>
#include <limits>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <build_cxx/common/abstract_target.hxx>
#include <build_cxx/common/file_target.hxx>
#include <build_cxx/common/phony_target.hxx>
#include <build_cxx/common/project.hxx>

#include "build_cxx/common/fs_proxy.hxx"
#include "build_cxx/driver/build_request.hxx"
#include "build_cxx/driver/dlopen_scoped.hxx"
#include "build_cxx/driver/processed_targets.hxx"
#include "build_cxx/driver/scheduler.hxx"

namespace build_cxx::driver {

void process_input(int const n_jobs,
                   std::vector<std::string_view> const &targets,
                   std::vector<std::string_view> const &priority_comparators,
                   std::vector<char const *> const &input_files) {
  std::vector<build_cxx::driver::dlopen_scoped> dl_handles;
  dl_handles.reserve(input_files.size());

  for (auto const input_file : input_files) {
    dl_handles.emplace_back(input_file);
  }

  // TODO "hide" this & related checks in relevant "processed_targets"s method
  std::unordered_map<std::string_view, std::string_view> processed_projects;

  auto comp_chain{make_comparator_chain(priority_comparators,
                                        common::fs_proxy::default_impl())};

  scheduler sched{build_request::comparator_inst{comp_chain.get()}, n_jobs};
  processed_targets pt{sched};

  bool all_resolved{false};

  for (auto const &dlh : dl_handles) {
    auto const proj{dlh.get_project()};

    if (proj == nullptr) {
      // TODO hard error?!
      continue;
    }

    auto const iter{processed_projects.find(proj->name)};

    if (iter == processed_projects.end()) {
      processed_projects.emplace(proj->name, proj->version);
    } else if (iter->second != proj->version) {
      throw std::runtime_error{
          "Request to load project '" + std::string{proj->name} +
          "' with various versions: '" + std::string{iter->second} + "' vs. '" +
          std::string{proj->version} + "'"};
    } else {
      std::cout << "Warning: project '" << proj->name << "' version '"
                << proj->version << "' already loaded\n";
      continue;
    }

    pt.process_project(proj);

    all_resolved = pt.resolve_deps_for_all();

    std::cout << "Project '" << proj->name << "' version '" << proj->version
              << "' processed; so far, " << (all_resolved ? "" : "NOT ")
              << "all dependencies are resolved\n";
  }

  if (!all_resolved) { // last value from the loop above
    throw std::runtime_error{
        "Not all dependencies could be resolved (or no input provided)"};
  }

  if (targets.empty()) {
    for (auto const [name, proj] : pt.projects_by_name) {
      // TODO build implicit target `all`:
      std::cout << "Project '" << proj->name << "' version '" << proj->version
                << "' has targets:\n";
      for (auto given_abstract_target{proj->first};
           given_abstract_target != nullptr;
           given_abstract_target = given_abstract_target->next) {
        std::cout << " - Target '" << given_abstract_target->name << "' ("
                  << given_abstract_target->resolved_name << ") is "
                  << given_abstract_target->resolved_kind << " target\n    ";
        given_abstract_target->recipe(
            pt.get_target_resolved_deps().at(given_abstract_target).deps);
        std::cout << '\n';
      }
      std::cout << '\n';
    }
  } else {
    pt.build_targets(targets, true);
  }
}

} // namespace build_cxx::driver
