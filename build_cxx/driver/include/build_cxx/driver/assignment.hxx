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

#include <string_view>
#include <thread>
#include <vector>

#include <build_cxx/common/macros.h>

namespace build_cxx::driver {

struct BUILD_CXX_DLL_EXPORT assignment {
  assignment() noexcept;
  ~assignment() noexcept;

  void process() const;

  int n_jobs{
      std::max(1, static_cast<int>(std::thread::hardware_concurrency()))};
  std::string_view build_cxx_file{"build.cxx"};
  std::vector<std::string_view> targets;
  std::vector<std::string_view> priority_comparators;

  // TODO ... this is bad, it is here only temporarily, for "proof of concept"
  // purposes:
  std::vector<std::string> input_files;

private:
  assignment(assignment const &) = delete;
  assignment &operator=(assignment const &) = delete;
  assignment(assignment &&) = delete;
  assignment &operator=(assignment &&) = delete;
};

} // namespace build_cxx::driver
