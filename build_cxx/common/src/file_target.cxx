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

file_target::file_target(location const *const loc, std::string_view const name,
                         std::string_view const *const raw_deps,
                         std::size_t const num_deps)
    : abstract_target{loc, name, raw_deps, num_deps} {
  // TODO ... is it really wanted in this form?!
  if (name[0] == '/') {
    resolved_path = std::filesystem::path{name};
  } else {
    std::filesystem::path listFolder{loc->aFilename};
    listFolder.remove_filename();

    resolved_path = listFolder / std::filesystem::path{name};
  }
}

abstract_target::modification_time_t
file_target::last_modification_time() const {
  try {
    auto const ftime{
        std::filesystem::last_write_time(resolved_path).time_since_epoch()};
    return static_cast<modification_time_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(ftime).count());
  } catch (std::filesystem::filesystem_error const &) {
    return never_up_to_date;
  }
}

} // namespace build_cxx::common
