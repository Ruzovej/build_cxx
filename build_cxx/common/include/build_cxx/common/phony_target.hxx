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

#include <string_view>

#include "build_cxx/common/abstract_target.hxx"
#include "build_cxx/common/macros.h"

namespace build_cxx::common {

struct BUILD_CXX_DLL_EXPORT phony_target : abstract_target {
  using abstract_target::abstract_target;

  // Phony target is always out of date
  [[nodiscard]] std::optional<modification_time_t>
  last_modification_time() const override {
    return std::nullopt;
  }

  static std::string resolve_name(std::string_view const project_name,
                                  std::string_view const target_name);

  static std::string_view constexpr kind{"phony"};

  void resolve_own_traits() override final;

  void build(std::vector<abstract_target const *> const &resolved_deps)
      override final {
    // nothing to manage ...
    recipe(resolved_deps);
  }

private:
  phony_target(phony_target const &) = delete;
  phony_target &operator=(phony_target const &) = delete;
  phony_target(phony_target &&) = delete;
  phony_target &operator=(phony_target &&) = delete;
};

} // namespace build_cxx::common
