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

#include "build_cxx/common/file_target.hxx"

#include <chrono>

namespace build_cxx::common {

file_target::file_target(location const *const loc, bool const include_in_all,
                         std::string_view const name,
                         std::string_view const *const raw_deps,
                         std::size_t const num_deps)
    : abstract_target{loc, include_in_all, name, raw_deps, num_deps} {}

abstract_target::modification_time_t
file_target::last_modification_time() const {
  if (!std::filesystem::exists(resolved_path)) {
    return never_up_to_date;
  }

  try {
    auto const ftime{
        std::filesystem::last_write_time(resolved_path).time_since_epoch()};
    return static_cast<modification_time_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(ftime).count());
  } catch (std::filesystem::filesystem_error const &) {
    return never_up_to_date;
  }
}

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

void file_target::resolve_own_name(std::string_view const /*project_name*/) {
  resolved_kind = "file";
  resolved_path = resolve_path(loc->filename, name);
  resolved_name = resolved_path.string();
}

} // namespace build_cxx::common
