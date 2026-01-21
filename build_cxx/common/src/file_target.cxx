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

#include <chrono>
#include <stdexcept>

namespace build_cxx::common {

abstract_target::modification_time_t
file_target::last_modification_time() const {
  if (!std::filesystem::exists(resolved_path)) {
    // throw std::runtime_error{
    //     "file_target::last_modification_time failed, because target file '" +
    //     resolved_path.string() + "' does not exist"};
    return std::numeric_limits<modification_time_t>::min();
  }

  auto const ftime{
      std::filesystem::last_write_time(resolved_path).time_since_epoch()};
  return static_cast<modification_time_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(ftime).count());
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

void file_target::resolve_own_traits() {
  resolved_kind = kind;
  resolved_path = resolve_path(loc->filename, name);
  resolved_name = resolved_path.string();
}

abstract_target::modification_time_t
read_only_file_target::last_modification_time() const {
  auto const my_mod_time{file_target::last_modification_time()};

  return std::max(highest_mod_time, my_mod_time);
}

void read_only_file_target::recipe(
    std::vector<abstract_target const *> const &resolved_deps) {
  for (auto const dep : resolved_deps) {
    auto const dep_mod_time{dep->last_modification_time()};

    highest_mod_time = std::max(highest_mod_time, dep_mod_time);
  }
}

} // namespace build_cxx::common
