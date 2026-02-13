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

#pragma once

#include <filesystem>
#include <string_view>

#include "build_cxx/common/abstract_target.hxx"
#include "build_cxx/common/fs_proxy.hxx"
#include "build_cxx/common/macros.h"

namespace build_cxx::common {

struct BUILD_CXX_DLL_EXPORT file_target : abstract_target {
  using abstract_target::abstract_target;

  static std::filesystem::path
  resolve_path(std::string_view const source_filename,
               std::string_view const target_name);

  static std::string_view constexpr kind{"file"};

  void resolve_own_traits() override final;

  void initialize_status() override;
  void update_status(target_status const newest_dep_status) override;

  std::filesystem::path const &get_resolved_path() const {
    return resolved_path;
  }

protected:
  std::filesystem::path resolved_path;

  fs_proxy *fs{fs_proxy::default_impl()};
};

struct BUILD_CXX_DLL_EXPORT read_only_file_target : file_target {
  using file_target::file_target;

  void recipe(std::vector<abstract_target const *> const &resolved_deps)
      const override {
    // read-only -> nothing to do ...
    static_cast<void>(resolved_deps);
  }

  void update_status(target_status const newest_dep_status) override;
};

} // namespace build_cxx::common
