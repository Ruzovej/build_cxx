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

#include <filesystem>
#include <fstream>
#include <iostream>

#include <build_cxx/client/core.hxx>
#include <build_cxx/client/debug_helper.hxx>

namespace {

void touch_file(std::filesystem::path const &p) {
  if (!std::filesystem::exists(p)) {
    auto const parent_path{p.parent_path()};
    if (!std::filesystem::exists(parent_path)) {
      std::filesystem::create_directories(parent_path);
    }
    std::ofstream ofs{p};
    ofs.close();
  } else {
    std::filesystem::last_write_time(
        p, std::filesystem::file_time_type::clock::now());
  }
}

auto const current_dir{
    std::filesystem::path{BUILD_CXX_CURRENT_LOCATION.filename}.parent_path()};

} // namespace

BUILD_CXX_PROJECT("BBB", "1.0.0");

BUILD_CXX_PHONY_TARGET("b_01",
                       // deps:
                       "b_02") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this,
                                                             resolved_deps);
}

BUILD_CXX_HIDDEN_PHONY_TARGET("b_02") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this,
                                                             resolved_deps);
}

// very fake ...:

BUILD_CXX_PHONY_TARGET("BBB",
                       // deps:
                       "bin/libBBB.a", "bin/libBBB.so") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this,
                                                             resolved_deps);
}

BUILD_CXX_FILE_TARGET("bin/libBBB.a",
                      // deps:
                      "build/src/BBB.cxx.o") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this,
                                                             resolved_deps);

  touch_file(current_dir / name);
}

BUILD_CXX_FILE_TARGET("bin/libBBB.so",
                      // deps:
                      "build/src/BBB.cxx.o") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this,
                                                             resolved_deps);

  touch_file(current_dir / name);
}

BUILD_CXX_HIDDEN_FILE_TARGET("build/src/BBB.cxx.o",
                             // deps:
                             "src/BBB.cxx") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this,
                                                             resolved_deps);

  touch_file(current_dir / name);
}

// simulating local file:
BUILD_CXX_HIDDEN_READ_ONLY_FILE_TARGET("src/BBB.cxx",
                                       // deps:
                                       "src/BBB.hxx");

BUILD_CXX_HIDDEN_READ_ONLY_FILE_TARGET("src/BBB.hxx",
                                       // deps:
                                       "/usr/include/string_view");

// simulating system file:
BUILD_CXX_HIDDEN_READ_ONLY_FILE_TARGET("/usr/include/string_view");
