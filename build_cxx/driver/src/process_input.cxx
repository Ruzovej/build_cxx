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

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <build_cxx/common/file_target.hxx>
#include <build_cxx/common/phony_target.hxx>
#include <build_cxx/common/project.hxx>

#include "build_cxx/common/abstract_target.hxx"
#include "build_cxx/driver/dlopen_scoped.hxx"
#include "build_cxx/driver/processed_targets.hxx"
namespace build_cxx::driver {

void process_input(std::vector<char const *> const &targets,
                   std::vector<char const *> const &input_files) {
  std::vector<build_cxx::driver::dlopen_scoped> dl_handles;
  dl_handles.reserve(input_files.size());

  for (auto const input_file : input_files) {
    dl_handles.emplace_back(input_file);
  }

  std::unordered_map<std::string_view, std::string_view> processed_projects;

  processed_targets pt{};

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

    all_resolved = pt.resolve_deps();

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
      // TODO real implementation:
      std::cout << "Project '" << proj->name << "' version '" << proj->version
                << "' has targets:\n";
      for (auto given_abstract_target{proj->first};
           given_abstract_target != nullptr;
           given_abstract_target = given_abstract_target->next) {
        std::cout << " - Target '" << given_abstract_target->name << "' ("
                  << given_abstract_target->resolved_name << ") is "
                  << given_abstract_target->resolved_kind << " target\n    ";
        given_abstract_target->build(
            pt.get_target_resolved_deps().at(given_abstract_target).deps);
        std::cout << '\n';
      }
      std::cout << '\n';
    }
  } else {
    // clang-format off
    // test by e.g.:
    // $ build/build_cxx/driver/build_cxx_driver build/tests/integration/lib01.so build/tests/integration/lib02.so --target BBB::b_01 --target AAA::a_phony_2
    // or
    // $ build/build_cxx/driver/build_cxx_driver build/tests/integration/lib03.so --target CCC::c1
    // clang-format on

    std::unordered_set<common::abstract_target const *> built_targets;

    std::function<void(common::abstract_target *, std::string const &)> const
        build_target = [&](common::abstract_target *const tgt,
                           std::string const &indent) {
          if (built_targets.find(tgt) != built_targets.cend()) {
            return;
          }

          std::cout << indent << "Building target '" << tgt->resolved_name
                    << "':\n";

          // TODO remove later ...
          if (tgt->name == "build/src/CCC.cxx.o") {
            [[maybe_unused]] volatile bool a = false;
            a = true;
          }

          auto const &deps{pt.get_target_resolved_deps().at(tgt).deps};

          auto highest_dep_mod_time{std::numeric_limits<
              common::abstract_target::modification_time_t>::min()};

          for (auto const dep : deps) {
            // TODO get rid of this ugly `const_cast` ...
            build_target(const_cast<common::abstract_target *>(dep),
                         indent + '\t');

            auto const dep_mod_time{dep->last_modification_time()};

            highest_dep_mod_time = std::max(highest_dep_mod_time, dep_mod_time);
          }

          auto const last_mod_time{tgt->last_modification_time()};

          std::cout << indent << "-> ";

          // `<` (may rebuild already up-to-date stuff) or `<=` (means `PHONY`
          // targets are up-to-date)?!
          // TODO figure out proper way later ...
          if (highest_dep_mod_time < last_mod_time) {
            std::cout << "is already up to date\n";
          } else {
            tgt->build(deps);
            std::cout << '\n';
          }

          built_targets.emplace(tgt);
        };

    for (std::string_view const target : targets) {
      auto const iter{pt.targets_by_resolved_name.find(target)};

      if (iter == pt.targets_by_resolved_name.cend()) {
        throw std::runtime_error{"Requested target '" + std::string{target} +
                                 "' not found"};
      }

      build_target(iter->second, "");
    }
  }
}

} // namespace build_cxx::driver
