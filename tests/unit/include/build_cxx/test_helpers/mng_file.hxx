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
#include <unordered_set>

namespace build_cxx::test_helpers {

struct mng_file {
  [[nodiscard]] static std::filesystem::path const &tmp_dir(bool const local);

  [[nodiscard]] static bool
  under_tmp_dir(std::filesystem::path const &filepath);

  ~mng_file();

  void touch(std::filesystem::path const &filepath);

  void rm(std::filesystem::path const &filepath);

private:
  std::unordered_set<std::filesystem::path> created_files;
};

} // namespace build_cxx::test_helpers
