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

#include "build_cxx/test_helpers/mng_file.hxx"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace build_cxx::test_helpers {

namespace {

std::filesystem::path const loc_tmp_dir{std::filesystem::absolute(
    std::filesystem::path{__FILE__}.parent_path().parent_path() / "tmp")};

std::filesystem::path const glob_tmp_dir{
    std::filesystem::absolute(std::filesystem::temp_directory_path())};

} // namespace

std::filesystem::path const &mng_file::tmp_dir(bool const local) {
  return local ? loc_tmp_dir : glob_tmp_dir;
}

bool mng_file::under_tmp_dir(std::filesystem::path const &filepath) {
  auto const abs_filepath{std::filesystem::absolute(filepath)};

  auto const under_tmp_dir_impl = [&](bool const local) {
    auto const &tmp_dir_path{tmp_dir(local)};

    return std::mismatch(tmp_dir_path.begin(), tmp_dir_path.end(),
                         abs_filepath.begin())
               .first == tmp_dir_path.end();
  };

  return under_tmp_dir_impl(true) || under_tmp_dir_impl(false);
}

mng_file::~mng_file() {
  for (auto const &filepath : created_files) {
    rm(filepath);
  }
}

void mng_file::touch(std::filesystem::path const &filepath) {
  if (filepath.empty()) {
    throw std::runtime_error{"Cannot touch an empty filepath"};
  } else if (filepath.c_str()[0] !=
             std::filesystem::path::preferred_separator) {
    return touch(tmp_dir(true) / filepath);
  } else if (!under_tmp_dir(filepath)) {
    throw std::runtime_error{
        "Cannot touch a file outside of the temporary directories: " +
        filepath.string()};
  }

  if (!std::filesystem::exists(filepath)) {
    auto const parent_path{filepath.parent_path()};

    if (!std::filesystem::exists(parent_path)) {
      std::filesystem::create_directories(parent_path);
    }

    std::ofstream{filepath}.close();

    created_files.emplace(filepath);
  } else {
    std::filesystem::last_write_time(
        filepath, std::filesystem::file_time_type::clock::now());
  }
}

void mng_file::rm(std::filesystem::path const &filepath) {
  if (created_files.count(filepath) != 0) {
    created_files.erase(filepath);
    std::filesystem::remove(filepath);
  }
}

} // namespace build_cxx::test_helpers
