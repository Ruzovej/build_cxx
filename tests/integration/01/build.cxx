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

#include <iostream>

#include <build_cxx/client/core.hxx>
#include <build_cxx/client/debug_helper.hxx>

BUILD_CXX_PROJECT("AAA", "1.0.0");

BUILD_CXX_PHONY_TARGET("a_phony_1",
                       // deps:
                       "a_phony_2", "a_phony_3", "bin/libAAA.a") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}

BUILD_CXX_PHONY_TARGET("a_phony_2",
                       // deps:
                       "BBB::BBB") {
  std::cout << "I'm twice as happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}

BUILD_CXX_HIDDEN_PHONY_TARGET("a_phony_3") {
  std::cout << "I'm happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}

// again, faking it a lot:

BUILD_CXX_FILE_TARGET("bin/libAAA.a",
                      // deps:
                      "a/src/AAA_1.cxx", "b/src/AAA_2.cxx",
                      "b/c/src/AAA_3.cxx") {
  std::cout << "I'm twice as happy :-) "
            << build_cxx::client::abstract_target_build_info(this, deps);
}

#include "a/build.cxx"
#include "b/build.cxx"
