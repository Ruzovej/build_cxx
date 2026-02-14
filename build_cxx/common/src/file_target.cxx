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

#include "build_cxx/common/file_target.hxx"

#include <stdexcept>

namespace build_cxx::common {

std::filesystem::path
file_target::resolve_path(std::string_view const source_filename,
                          std::string_view const target_name) {
  std::filesystem::path resolved_path;

  if (target_name.at(0) == '/') {
    return target_name;
  } else {
    std::filesystem::path listFolder{source_filename};
    listFolder.remove_filename();

    return listFolder / std::filesystem::path{target_name};
  }
}

void file_target::resolve_own_traits() {
  resolved_kind = kind;
  resolved_path = resolve_path(loc->filename, name);
  resolved_name = resolved_path.string();
}

void file_target::initialize_status() {
  status = fs->file_exists(resolved_path)
               ? target_status{fs->file_last_mod_time(resolved_path)}
               : target_status::explicitly_needs_update;
}

void file_target::update_status(target_status const newest_dep_status) {
  // this target should be directly mapped to a file -> it has status
  // independent on its deps.; TODO later check that if this "newest_dep_status"
  // is "timepoint"-like, it's <= than own. mod file
  static_cast<void>(newest_dep_status);

  if (!fs->file_exists(resolved_path)) {
    throw std::runtime_error{"After running its recipe, expected file '" +
                             resolved_name + "' doesn't exist"};
  }

  status = target_status{fs->file_last_mod_time(resolved_path)};
}

void read_only_file_target::update_status(
    target_status const newest_dep_status) {
  status.merge_with(newest_dep_status);
}

} // namespace build_cxx::common
