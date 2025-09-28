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

#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "build_cxx/targets/generic.hxx"

namespace build_cxx::targets {

class translation_unit : public generic {
public:
  explicit translation_unit(std::string &&name) : generic{std::move(name)} {}

  virtual ~translation_unit() = default;

  virtual bool up_to_date() const = 0;
  virtual void build() const = 0;

private:
  std::string source_file_dir;
  std::string source_file;
  std::string object_file_dir;

  std::vector<std::string> extra_compiler_flags;
};

} // namespace build_cxx::targets
