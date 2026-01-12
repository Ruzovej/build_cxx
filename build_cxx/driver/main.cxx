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

#include <exception>
#include <iostream>
#include <vector>

#include "build_cxx/driver/process_input.hxx"

int main(int argc, char *argv[]) {
  try {
    std::vector<char const *> input_files;
    input_files.reserve(argc - 1);

    --argc;
    ++argv;
    while (argc > 0) {
      auto const arg{argv[0]};
      --argc;
      ++argv;
      input_files.emplace_back(arg); // TODO refuse already processed *.so ...
    }

    build_cxx::driver::process_input(input_files);

    return EXIT_SUCCESS;
  } catch (std::exception const &e) {
    std::cerr << "build_cxx_driver failed - error (exception): " << e.what()
              << "\n";
  } catch (char const *const msg) {
    std::cerr << "build_cxx_driver failed - error (char const *): " << msg
              << "\n";
  } catch (...) {
    std::cerr << "build_cxx_driver failed - unknown error\n";
  }
  return EXIT_FAILURE;
}
