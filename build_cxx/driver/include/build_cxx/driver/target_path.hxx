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

#pragma once

#include <string>
#include <string_view>

namespace build_cxx::driver {

struct target_path {
  std::string_view project_name;
  std::string file_name;
  std::string target_name;

  [[nodiscard]] bool operator==(target_path const &other) const {
    validate();
    // TODO simplify using `std::tie`, etc.:
    return (project_name == other.project_name) &&
           (file_name == other.file_name) && (target_name == other.target_name);
  }

  [[nodiscard]] bool operator!=(target_path const &other) const {
    return !(*this == other);
  }

  struct hasher {
    [[nodiscard]] std::size_t operator()(target_path const &tp) const noexcept {
      // TODO better ...
      return std::hash<std::string_view>{}(tp.project_name) ^
             std::hash<std::string>{}(tp.file_name) ^
             std::hash<std::string>{}(tp.target_name);
    }
  };

private:
  void validate() const {
    // TODO better exception type
    if (target_name.empty()) {
      throw "Invalid target_path - target_name must not be empty";
    }
    if (project_name.empty() == file_name.empty()) {
      throw "Invalid target_path - exactly one of project_name/file_name must "
            "be empty";
    }
  }
};

} // namespace build_cxx::driver
