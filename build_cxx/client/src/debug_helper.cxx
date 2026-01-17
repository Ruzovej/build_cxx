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

#include "build_cxx/client/debug_helper.hxx"

namespace build_cxx::client {

std::string abstract_target_basic_info(common::abstract_target const *const at,
                                       bool const brief) {
  std::string res{at->name};

  if (!brief) {
    res += " defined in '" + std::string{at->loc->filename} + ':' +
           std::to_string(at->loc->line) + '\'';
  }

  return res;
}

std::string abstract_target_build_info(
    common::abstract_target const *const at,
    std::vector<common::abstract_target const *> const &deps) {
  std::string res{abstract_target_basic_info(at, true) + " has deps {"};

  bool first{true};
  for (auto const dep : deps) {
    if (!first) {
      res += ", ";
    } else {
      first = false;
    }

    // res += abstract_target_basic_info(dep, false);
    res += dep->resolved_name;
  }

  return res + '}';
}

} // namespace build_cxx::client
