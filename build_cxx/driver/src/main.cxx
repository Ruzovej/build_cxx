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

#include <dlfcn.h>

#include <cstdlib>

#include <exception>
#include <iostream>
#include <vector>

#include <build_cxx/common/file_target.hxx>
#include <build_cxx/common/phony_target.hxx>
#include <build_cxx/common/project.hxx>

namespace {

struct dlopen_scoped {
  explicit dlopen_scoped(char const *const filename)
      : handle{dlopen(filename, RTLD_LAZY /*| RTLD_GLOBAL*/)} {
    if (handle == nullptr) {
      throw std::runtime_error{std::string{"dlopen failed - error: "} +
                               dlerror()};
    }
  }

  build_cxx::common::project *get_project() const {
    static auto constexpr symbol_name{"build_cxx_get_project"};

    auto symbol{dlsym(handle, symbol_name)};
    if (symbol == nullptr) { // not mandatory ... TODO later make it so!
      return nullptr;
    }

    using get_proj_fn = build_cxx::common::project *();

    auto const fn{reinterpret_cast<get_proj_fn *>(symbol)};

    return fn();
  }

  ~dlopen_scoped() noexcept {
    if (handle != nullptr) {
      if (dlclose(handle) < 0) {
        std::cerr << "Warning: dlclose failed - error: " << dlerror() << '\n';
      }
    }
  }

private:
  void *handle;
};

} // namespace

int main(int argc, char *argv[]) {
  try {
    std::vector<dlopen_scoped> dl_handles;
    dl_handles.reserve(argc - 1);

    --argc;
    ++argv;
    while (argc > 0) {
      auto const arg{argv[0]};
      --argc;
      ++argv;
      dl_handles.emplace_back(arg); // TODO refuse already processed *.so ...
    }

    for (auto &dlh : dl_handles) {
      auto const proj{dlh.get_project()};
      if (proj != nullptr) {
        std::cout << "Project '" << proj->name << "' version '" << proj->version
                  << "' has targets:\n";
      }

      for (auto at{proj->first}; at != nullptr; at = at->next) {
        std::cout << " - Target '" << at->name << "'\n";
        auto const phony{dynamic_cast<build_cxx::common::phony_target *>(at)};
        if (phony) {
          std::cout << "    - is phony target: ";
          phony->build();
        }
      }
      std::cout << '\n';
    }

    return EXIT_SUCCESS;
  } catch (std::exception const &e) {
    std::cerr << "build_cxx::driver failed - error (exception): " << e.what()
              << "\n";
  } catch (char const *const msg) {
    std::cerr << "build_cxx::driver failed - error (char const *): " << msg
              << "\n";
  } catch (...) {
    std::cerr << "build_cxx::driver failed - unknown error\n";
  }
  return EXIT_FAILURE;
}
