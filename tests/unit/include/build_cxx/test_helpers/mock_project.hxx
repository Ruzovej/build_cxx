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
#include <unordered_set>
#include <vector>

#include "build_cxx/common/location.hxx"
#include "build_cxx/common/project.hxx"
#include "build_cxx/test_helpers/built_targets_t.hxx"
#include "build_cxx/test_helpers/mock_file_target.hxx"
#include "build_cxx/test_helpers/mock_phony_target.hxx"

namespace build_cxx::test_helpers {

struct mock_project : common::project {
  using project::project;

  [[nodiscard]] mock_file_target *add_mock_file_target(
      std::string_view const fake_loc_filename, bool const include_in_all,
      std::string_view const tgt_name, std::vector<std::string_view> &&deps) {
    return add_mock_target<mock_file_target>(fake_loc_filename, include_in_all,
                                             tgt_name, std::move(deps));
  }

  [[nodiscard]] mock_phony_target *add_mock_phony_target(
      std::string_view const fake_loc_filename, bool const include_in_all,
      std::string_view const tgt_name, std::vector<std::string_view> &&deps) {
    return add_mock_target<mock_phony_target>(fake_loc_filename, include_in_all,
                                              tgt_name, std::move(deps));
  }

  // testing helper ...
  built_targets_t *built_targets{nullptr};

private:
  struct target_holder {
    common::location fake_loc;
    std::vector<std::string_view> deps;
    std::unique_ptr<common::abstract_target> tgt;
  };

  std::vector<target_holder> fake_tgts;

  template <typename target_t>
  [[nodiscard]] target_t *
  add_mock_target(std::string_view const fake_loc_filename,
                  bool const include_in_all, std::string_view const tgt_name,
                  std::vector<std::string_view> &&deps) {
    auto const fake_ind{static_cast<int>(fake_tgts.size() + 1)};

    fake_tgts.emplace_back(
        target_holder{common::location{fake_loc_filename, 42, fake_ind},
                      std::move(deps), nullptr});

    auto &last{fake_tgts.back()};

    auto uptr{std::make_unique<target_t>(&last.fake_loc, include_in_all,
                                         tgt_name, last.deps.data(),
                                         last.deps.size())};

    add_target(uptr.get());

    uptr->built_targets = built_targets;

    auto *const ret{uptr.get()};

    last.tgt = std::move(uptr);

    return ret;
  }
};

} // namespace build_cxx::test_helpers
