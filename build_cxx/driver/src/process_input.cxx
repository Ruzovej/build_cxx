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

#include "build_cxx/driver/process_input.hxx"

#include <iostream>
#include <string_view>
#include <unordered_map>

#include <build_cxx/common/file_target.hxx>
#include <build_cxx/common/phony_target.hxx>
#include <build_cxx/common/project.hxx>

#include "build_cxx/driver/dlopen_scoped.hxx"
#include "build_cxx/driver/processed_targets.hxx"

namespace build_cxx::driver {

void process_input(std::vector<char const *> const &input_files) {
  std::vector<build_cxx::driver::dlopen_scoped> dl_handles;
  dl_handles.reserve(input_files.size());

  for (auto const input_file : input_files) {
    dl_handles.emplace_back(input_file);
  }

  std::unordered_map<std::string_view, std::string_view> processed_projects;

  processed_targets pt;

  bool all_resolved{false};

  for (auto const &dlh : dl_handles) {
    auto const proj{dlh.get_project()};

    if (proj == nullptr) {
      // TODO hard error?!
      continue;
    }

    auto const iter = processed_projects.find(proj->name);

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

    all_resolved = pt.resolve_deps();

    std::cout << "Project '" << proj->name << "' version '" << proj->version
              << "' processed; so far, " << (all_resolved ? "" : "NOT ")
              << "all dependencies are resolved\n";
  }

  if (!all_resolved) { // last value from the loop above
    throw std::runtime_error{
        "Not all dependencies could be resolved (or no input provided)"};
  }

  for (auto const &dlh : dl_handles) {
    auto const proj{dlh.get_project()};

    // TODO real implementation:
    std::cout << "Project '" << proj->name << "' version '" << proj->version
              << "' has targets:\n";
    for (auto at{proj->first}; at != nullptr; at = at->next) {
      std::cout << " - Target '" << at->name << "'\n";

      auto const phony{dynamic_cast<build_cxx::common::phony_target *>(at)};
      if (phony) {
        std::cout << "    - is phony target: ";
        phony->build(pt.get_target_resolved_deps().at(phony).deps);
        std::cout << '\n';
        continue;
      }

      auto const file_like{dynamic_cast<build_cxx::common::file_target *>(at)};
      if (file_like) {
        std::cout << "    - is file target: ";
        file_like->build(pt.get_target_resolved_deps().at(file_like).deps);
        std::cout << '\n';
        continue;
      }
    }
    std::cout << '\n';
  }
}

} // namespace build_cxx::driver
