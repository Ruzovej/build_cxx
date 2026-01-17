/*
  Copyright 2025 Lukáš Růžička

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

#include <iostream>

#include <build_cxx/client/core.hxx>
#include <build_cxx/client/debug_helper.hxx>

BUILD_CXX_PROJECT("BBB", "1.0.0");

BUILD_CXX_PHONY_TARGET("b_01",
                       // deps:
                       "b_02") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}

BUILD_CXX_HIDDEN_PHONY_TARGET("b_02") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}

// very fake ...:

BUILD_CXX_PHONY_TARGET("BBB",
                       // deps:
                       "bin/libBBB.a", "bin/libBBB.so") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}

BUILD_CXX_FILE_TARGET("bin/libBBB.a",
                      // deps:
                      "build/src/BBB.cxx.o") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}

BUILD_CXX_FILE_TARGET("bin/libBBB.so",
                      // deps:
                      "build/src/BBB.cxx.o") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}

BUILD_CXX_HIDDEN_FILE_TARGET("build/src/BBB.cxx.o",
                             // deps:
                             "src/BBB.cxx", "/usr/include/string_view") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}

// simulating local file:
BUILD_CXX_HIDDEN_FILE_TARGET("src/BBB.cxx") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}

// simulating system file:
BUILD_CXX_HIDDEN_FILE_TARGET("/usr/include/string_view") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}
