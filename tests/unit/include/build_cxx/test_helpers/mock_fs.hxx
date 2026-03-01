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
#include <mutex>
#include <unordered_map>

#include "build_cxx/common/fs_proxy.hxx"
#include "build_cxx/test_helpers/fake_clock.hxx"

namespace build_cxx::test_helpers {

struct mock_fs final : common::fs_proxy {
  explicit mock_fs(std::mutex *const aMtx = nullptr)
      : common::fs_proxy{}, mtx{aMtx} {
    // force 2 lines
  }

  [[nodiscard]] std::filesystem::path tmp_dir() const override {
    return "/fake/tmp";
  }

  [[nodiscard]] bool
  file_exists(std::filesystem::path const &path) const override {
    auto const lck{lock()};

    return files.find(path) != files.cend();
  }

  [[nodiscard]] common::target_status::file_mod_time_t
  file_last_mod_time(std::filesystem::path const &path) const override {
    auto const lck{lock()};

    return files.at(path).last_mod_time;
  }

  void touch(std::filesystem::path const &path) override {
    if (path.empty()) {
      throw std::runtime_error{"Can't touch an empty filepath"};
    }

    auto const lck{lock()};

    files[path].last_mod_time = clock.now_ns();
  }

  void rm(std::filesystem::path const &path) override {
    auto const lck{lock()};

    auto const iter{files.find(path)};

    if (iter != files.end()) {
      files.erase(iter);
    }
  }

  [[nodiscard]] bool empty() const {
    auto const lck{lock()};

    return files.empty();
  }

  void reset() {
    auto const lck{lock()};

    clock.reset();
    files.clear();
  }

  fake_clock clock;

private:
  struct fake_file {
    common::target_status::file_mod_time_t last_mod_time{};
    // std::string content; // TODO ...?!
  };

  [[nodiscard]] std::unique_lock<std::mutex> lock() const {
    if (mtx) {
      return std::unique_lock<std::mutex>{*mtx};
    }

    return {};
  }

  std::mutex *mtx{nullptr};
  std::unordered_map<std::filesystem::path, fake_file> files;
};

} // namespace build_cxx::test_helpers
