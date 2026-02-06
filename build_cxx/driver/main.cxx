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

#include <exception>
#include <iostream>
#include <thread>
#include <vector>

#include "build_cxx/driver/process_input.hxx"

int main(int argc, char *argv[]) {
  auto consume_arg = [&argc, &argv](bool const mandatory,
                                    std::string_view const err_msg =
                                        "") -> char const * {
    if (argc <= 0) {
      if (mandatory) {
        throw std::runtime_error("Insufficient arguments: " +
                                 std::string{err_msg});
      } else {
        return nullptr;
      }
    } else {
      char const *const res{argv[0]};
      --argc;
      ++argv;
      return res;
    }
  };

  try {
    std::vector<char const *> targets;
    std::vector<char const *> input_files;
    auto n_jobs{
        std::max(1, static_cast<int>(std::thread::hardware_concurrency()))};

    // skip executable name ...
    static_cast<void>(consume_arg(true, "missing executable filename"));

    while (argc > 0) {
      auto const next_arg_cstr{consume_arg(true, "argc mismatch")};
      std::string_view const next_arg{next_arg_cstr};

      if (next_arg == "--target" || next_arg == "-t") {
        targets.emplace_back(
            consume_arg(true, "missing target name after --target/-t"));
      } else if (next_arg == "--jobs" || next_arg == "-j") {
        auto const n_jobs_str{
            consume_arg(true, "missing number after --jobs/-j")};
        try {
          n_jobs = std::stoi(std::string{n_jobs_str});
        } catch (...) {
          throw std::runtime_error{"Invalid number for --jobs/-j: " +
                                   std::string{n_jobs_str}};
        }
      } else {
        input_files.emplace_back(next_arg_cstr);
      }
    }

    build_cxx::driver::process_input(n_jobs, targets, input_files);

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
