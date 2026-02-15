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

#pragma once

#include <queue>

#include "build_cxx/driver/build_request.hxx"
#include "build_cxx/driver/build_request_comparator.hxx"

namespace build_cxx::driver {

using build_request_priority_queue =
    std::priority_queue<build_request, std::vector<build_request>,
                        build_request_comparator>;

} // namespace build_cxx::driver
