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

#include <cli11_wrapper/args.hxx>

namespace build_cxx::system_tests {

struct env {
  [[nodiscard]] static env const &inst() noexcept;

  // consume known arguments, leave there the rest
  [[nodiscard]] static int setup(cli11_wrapper::args &args);

  // values:
  std::string build_cxx_driver_path;
  std::string build_cxx_repo_root;
  std::string build_cxx_system_test_cases_root;

  // TODO

private:
  env() noexcept;
  ~env() noexcept;

  [[nodiscard]] static env &inst_priv() noexcept;

  [[nodiscard]] int do_setup(cli11_wrapper::args &args);

  env(const env &) = delete;
  env &operator=(const env &) = delete;
  env(env &&) = delete;
  env &operator=(env &&) = delete;

  bool initialized{false};
};

} // namespace build_cxx::system_tests
