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

#include "build_cxx/common/abstract_target.hxx"

//#include <limits>
//#include <stdexcept>

//#include "build_cxx/common/proxy.hxx"

namespace build_cxx::common {

abstract_target::abstract_target(location const *const aLoc,
                                 std::string_view const aName,
                                 std::string_view const *const aDeps,
                                 std::size_t const aNum_deps)
    : loc{aLoc}, name{aName}, deps{aDeps}, num_deps{aNum_deps} {}

// bool abstract_target::is_up_to_date() const {
//   auto const my_modification_time{last_modification_time()};
//
//   for (std::size_t i{0}; i < num_deps; ++i) {
//     proxy const dep_proxy{"", deps[i]};
//
//     auto const dep_target{dep_proxy.get_target()};
//
//     if (dep_target == nullptr) {
//       throw std::runtime_error{"failed to resolve dependency '" +
//                                std::string{deps[i]} + "' for target '" +
//                                std::string{name} + "'"};
//     }
//
//     if (!dep_target->is_up_to_date() ||
//         (my_modification_time < dep_target->last_modification_time())) {
//       return false;
//     }
//   }
//
//   return my_modification_time < std::numeric_limits<long long>::max();
// }

} // namespace build_cxx::common
