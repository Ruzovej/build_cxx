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

#include "build_cxx/common/target_status.hxx"

namespace build_cxx::common {

struct fs_proxy {
  [[nodiscard]] static fs_proxy *default_impl();

  virtual ~fs_proxy() noexcept = default;

  [[nodiscard]] virtual std::filesystem::path tmp_dir() const = 0;

  [[nodiscard]] virtual bool
  file_exists(std::filesystem::path const &path) const = 0;

  [[nodiscard]] virtual target_status::file_modification_time_t
  file_last_mod_time(std::filesystem::path const &path) const = 0;

  virtual void touch(std::filesystem::path const &path) = 0;

  virtual void rm(std::filesystem::path const &path) = 0;
};

} // namespace build_cxx::common
