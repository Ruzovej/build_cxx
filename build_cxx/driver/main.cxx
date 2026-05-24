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

#include <cli11_wrapper/argv_parser.hxx>

#include "build_cxx/driver/assignment.hxx"

int main(int argc, char *argv[]) {
  try {
    build_cxx::driver::assignment assignment{};

    cli11_wrapper::argv_parser parser{"TODO app desc.",
                                      "TODO app version",
                                      argv[0],
                                      {
                                          // TODO config names
                                      },
                                      argc,
                                      argv};

    parser.set_allow_extras(true);

    parser.add_option(
        "-j,--jobs", assignment.n_jobs,
        "number of parallel jobs (defaults to number of CPU cores)");

    parser.add_option("-B,--Build,--build-cxx-file", assignment.build_cxx_file,
                      "path to the build.cxx file to use (defaults to "
                      "build.cxx in the current directory)");

    parser.add_option("-c,--comparator", assignment.priority_comparators,
                      "priority comparator(s) to use (can be specified "
                      "multiple times to build a chain of comparators)");

    parser.add_option(
        "-i,--input", assignment.input_files,
        "input file(s) to process (can be specified multiple times)");

    CLI11_WRAPPER_PARSE(parser);

    assignment.targets = parser.view_parsed_extras();

    assignment.process();

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
