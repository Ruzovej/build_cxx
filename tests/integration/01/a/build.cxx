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

BUILD_CXX_FILE_TARGET("src/AAA_1.cxx") {
  std::cout << "I'm twice as happy :-) "
            << build_cxx::client::abstract_target_build_info(this,
                                                             resolved_deps);

  touch_file(current_dir / name);
}
