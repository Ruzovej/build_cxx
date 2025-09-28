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

namespace build_cxx::targets {

class generic : public std::enable_shared_from_this<generic> {
protected:
  explicit generic(std::string &&name)
      : enable_shared_from_this(), name(std::move(name)) {}

public:
  template <typename T, typename... Args>
  static std::shared_ptr<generic> create(std::string &&name, Args &&...args) {
    using cleaned_T = std::remove_cv_t<T>;
    static_assert(std::is_base_of_v<generic, cleaned_T>,
                  "T must derive from build_cxx::targets::generic");
    static_assert(
        std::is_constructible_v<cleaned_T, std::string &&,
                                decltype(std::forward<Args>(args))...>,
        "T must be constructible from (std::string &&, Args...)");
    return std::shared_ptr<generic>(
        new cleaned_T{std::move(name), std::forward<Args>(args)...});
  }
  virtual ~generic() = default;

  virtual bool up_to_date() const = 0;
  virtual void build() const = 0;

  void register_dependency(std::shared_ptr<generic> other) {
    other->needed_by.emplace_back(weak_from_this());
    needs.emplace_back(std::move(other));
  }

  std::vector<std::shared_ptr<generic>> const &get_needs() const noexcept {
    return needs;
  }
  std::vector<std::weak_ptr<generic>> const &get_needed_by() const noexcept {
    return needed_by;
  }

private:
  generic(generic const &) = delete;
  generic &operator=(generic const &) = delete;
  generic(generic &&) = delete;
  generic &operator=(generic &&) = delete;

  std::vector<std::shared_ptr<generic>> needs;
  std::vector<std::weak_ptr<generic>> needed_by;
  std::string const name;
};

} // namespace build_cxx::targets
