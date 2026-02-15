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

#include <memory>
#include <queue>
#include <string_view>
#include <vector>

#include <build_cxx/common/fs_proxy.hxx>
#include <build_cxx/common/macros.h>

#include "build_cxx/driver/build_request.hxx"

namespace build_cxx::driver {

// TODO ... EXPORT or HIDE?!
struct BUILD_CXX_DLL_EXPORT comparator {
  virtual ~comparator() = default;

  [[nodiscard]] bool operator()(build_request const &lhs,
                                build_request const &rhs) const {
    auto const res{compare(lhs, rhs)};

    if (res < 0) {
      return true;
    } else if (res == 0) {
      return (next != nullptr) ? (*next)(lhs, rhs) : false;
    } else {
      return false;
    }
  }

  std::unique_ptr<comparator> next{nullptr};

protected:
  // ret = -1 -> lhs < rhs; ret = 0 -> equal; ret = 1 -> rhs < lhs
  [[nodiscard]] virtual int compare(build_request const &lhs,
                                    build_request const &rhs) const = 0;
};

// TODO ... EXPORT or HIDE?!
struct BUILD_CXX_DLL_EXPORT comparator_inst {
  explicit comparator_inst(comparator *aCmp) : cmp{aCmp} {
    // force 2 lines
  }

  [[nodiscard]] bool operator()(build_request const &lhs,
                                build_request const &rhs) const {
    // https://en.cppreference.com/w/cpp/container/priority_queue.html ... to
    // prevent confusion: the lowest value means being processed last by the
    // priority queue, so we need to invert the result of comparison:
    return (*cmp)(rhs, lhs);
  }

private:
  // "hairy": not owning it, and must be `!= nullptr`:
  comparator *cmp;
};

} // namespace build_cxx::driver
