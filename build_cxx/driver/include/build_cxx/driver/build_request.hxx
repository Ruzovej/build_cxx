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

#include <build_cxx/common/abstract_target.hxx>
#include <build_cxx/common/fs_proxy.hxx>
#include <build_cxx/common/macros.h>

namespace build_cxx::driver {

// TODO ... EXPORT or HIDE?! What about its sub-structs?
struct BUILD_CXX_DLL_EXPORT build_request {
  common::abstract_target *tgt{nullptr};
  std::vector<common::abstract_target const *> const *deps{nullptr};

  struct comparator {
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

  struct comparator_inst {
    explicit comparator_inst(comparator *aCmp) : cmp{aCmp} {
      // force 2 lines
    }

    [[nodiscard]] bool operator()(build_request const &lhs,
                                  build_request const &rhs) const {
      // https://en.cppreference.com/w/cpp/container/priority_queue.html ... to
      // prevent confusion: the lowest value means being processed last by the
      // priority queue, so we need to invert the result of comparison:
      return !(*cmp)(lhs, rhs);
    }

  private:
    // "hairy": not owning it, and must be `!= nullptr`:
    comparator *cmp;
  };
};

namespace sort_by {

static std::string_view constexpr name_asc{"name_asc"};
static std::string_view constexpr name_desc{"name_desc"};
static std::string_view constexpr mod_time_asc{"mod_time_asc"};
static std::string_view constexpr mod_time_desc{"mod_time_desc"};

} // namespace sort_by

[[nodiscard]] std::unique_ptr<build_request::comparator>
make_comparator_chain(std::vector<std::string_view> const &comparator_names,
                      common::fs_proxy *const fs);

using build_request_priority_queue =
    std::priority_queue<build_request, std::vector<build_request>,
                        build_request::comparator_inst>;

} // namespace build_cxx::driver
