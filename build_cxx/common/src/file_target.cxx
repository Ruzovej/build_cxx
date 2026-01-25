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
#include <limits>
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
  initialize_status();
}

void file_target::update_status() { initialize_status(); }

namespace {

target_status::file_modification_time_t
file_last_modification_time(std::filesystem::path const &path) {
  auto const ftime{std::filesystem::last_write_time(path).time_since_epoch()};

  return static_cast<target_status::file_modification_time_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(ftime).count());
}

} // namespace

target_status file_target::compute_status() const {
  return target_status{file_last_modification_time(resolved_path)};
}

bool file_target::exists() const {
  return std::filesystem::exists(resolved_path);
}

void file_target::initialize_status() {
  status = exists() ? compute_status() : target_status::needs_update;
}

/*
void file_target::post_recipe(
    std::optional<modification_time_t> const &highest_dep_mod_time) {
  if (!exists()) {
    throw std::runtime_error{
        "unmet post-condition after running `recipe`: target file '" +
        resolved_path.string() + "' doesn't exist"};
  } else if (highest_dep_mod_time.has_value() &&
             (file_last_modification_time(resolved_path) <
              *highest_dep_mod_time)) {
    throw std::runtime_error{
        "unmet post-condition after running `recipe`: target file '" +
        resolved_path.string() + "' isn't newer than its newest dependency"};
  }
}

std::optional<abstract_target::modification_time_t>
read_only_file_target::last_modification_time() const {
  if (!highest_mod_time.has_value()) {
    return std::nullopt;
  }

  auto const my_last_mod_time{file_target::last_modification_time()};

  if (!my_last_mod_time.has_value()) {
    return std::nullopt;
  }

  return std::max(*highest_mod_time, *my_last_mod_time);
}

void read_only_file_target::post_recipe(
    std::optional<modification_time_t> const &highest_dep_mod_time) {
  highest_mod_time = highest_dep_mod_time;
}*/

} // namespace build_cxx::common
