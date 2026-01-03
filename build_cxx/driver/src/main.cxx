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

#pragma once

#include <cstdlib>

#include <exception>
#include <iostream>

#include "build_cxx/impl/target_builder.hxx"

#ifndef BUILD_CXX_INTERNAL_IMPLEMENTATION

int main(int const argc, char const *const argv[]) {
  try {
    auto &target_builders = ::build_cxx::impl::get_target_builders_vector();

    for (auto &tb : target_builders) {
      // tb.fn(const_cast<::build_cxx::impl::target_builder &>(tb));
      std::cout << "Processing target '" << tb.name << "' defined at '"
                << tb.filename << ":" << tb.line << "' (index " << tb.index
                << "):\n";

      tb.adjust_target();
    }

    return EXIT_SUCCESS;
  } catch (std::exception const &e) {
    std::cerr << "build_cxx failed - error (exception): " << e.what() << "\n";
  } catch (char const *const msg) {
    std::cerr << "build_cxx failed - error (char const *): " << msg << "\n";
  } catch (...) {
    std::cerr << "build_cxx failed - unknown error\n";
  }
  return EXIT_FAILURE;
}

#endif
