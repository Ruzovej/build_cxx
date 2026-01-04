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

BUILD_CXX_PROJECT("AAA", "1.0.0");

BUILD_CXX_PHONY_TARGET("AAA 1st target") {
  std::cout << "I'm happy :-) - inside target '" << current_target.name
            << "'\n";
}

BUILD_CXX_GENERIC_TARGET("Root target") {
  std::cout << "I'm happy :-) - inside '" << current_target.name << "'\n";
}

#include "a/build.cxx"
#include "b/build.cxx"
