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
#include <limits>
#include <string_view>

#include "build_cxx/common/abstract_target.hxx"
#include "build_cxx/common/macros.h"

namespace build_cxx::common {

struct BUILD_CXX_DLL_EXPORT file_target : abstract_target {
  using abstract_target::abstract_target;

  [[nodiscard]] modification_time_t last_modification_time() const override;

  static std::filesystem::path
  resolve_path(std::string_view const source_filename,
               std::string_view const target_name);

  static std::string_view constexpr kind{"file"};

  void resolve_own_traits() override final;

private:
  file_target(file_target const &) = delete;
  file_target &operator=(file_target const &) = delete;
  file_target(file_target &&) = delete;
  file_target &operator=(file_target &&) = delete;

  std::filesystem::path resolved_path;
};

struct BUILD_CXX_DLL_EXPORT read_only_file_target : file_target {
  using file_target::file_target;

  [[nodiscard]] modification_time_t last_modification_time() const override;

  void build(std::vector<abstract_target const *> const &deps) override;

private:
  modification_time_t highest_mod_time{
      std::numeric_limits<modification_time_t>::min()};
};

} // namespace build_cxx::common
