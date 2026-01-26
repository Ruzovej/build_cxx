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

#include "build_cxx/common/fs_proxy.hxx"

#include <fstream>
#include <stdexcept>

namespace build_cxx::common {

namespace {

struct dflt_impl : fs_proxy {
  [[nodiscard]] std::filesystem::path tmp_dir(bool const local) const override {
    return std::filesystem::temp_directory_path();
  }

  [[nodiscard]] bool
  file_exists(std::filesystem::path const &path) const override {
    return std::filesystem::exists(path);
  }

  [[nodiscard]] target_status::file_modification_time_t
  file_last_mod_time(std::filesystem::path const &path) const override {
    auto const ftime{std::filesystem::last_write_time(path).time_since_epoch()};

    return static_cast<target_status::file_modification_time_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(ftime).count());
  }

  void touch(std::filesystem::path const &filepath) override {
    if (filepath.empty()) {
      throw std::runtime_error{"Can't touch an empty filepath"};
    }

    if (!std::filesystem::exists(filepath)) {
      auto const parent_path{filepath.parent_path()};

      if (!std::filesystem::exists(parent_path)) {
        std::filesystem::create_directories(parent_path);
      }

      std::ofstream{filepath}.close();
    } else {
      std::filesystem::last_write_time(
          filepath, std::filesystem::file_time_type::clock::now());
    }
  }

  void rm(std::filesystem::path const &filepath) override {
    if (!std::filesystem::remove(filepath)) {
      throw std::runtime_error{"Failed to remove file '" + filepath.string() +
                               '\''};
    }
  }
};

dflt_impl inst{};

} // namespace

fs_proxy *fs_proxy::default_impl() { return &inst; }

fs_proxy::~fs_proxy() noexcept = default;

} // namespace build_cxx::common
