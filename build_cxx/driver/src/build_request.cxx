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

#include "build_cxx/driver/build_request.hxx"
#include "build_cxx/driver/build_request_comparator.hxx"
#include "build_cxx/driver/make_comparator_chain.hxx"

#include <build_cxx/common/file_target.hxx>

namespace build_cxx::driver {

namespace {

template <typename T> int signum(T val) {
  if (val < 0) {
    return -1;
  } else if (val > 0) {
    return 1;
  } else {
    return 0;
  }
}

template <bool asc> struct name_cmp final : comparator {
  // ret = -1 -> lhs < rhs; ret = 0 -> equal; ret = 1 -> rhs < lhs
  [[nodiscard]] int compare(build_request const &lhs,
                            build_request const &rhs) const override {
    std::string_view const lhs_name{lhs.tgt->resolved_name};
    std::string_view const rhs_name{rhs.tgt->resolved_name};

    return asc ? lhs_name.compare(rhs_name) : rhs_name.compare(lhs_name);
  }
};

template <bool asc> struct mod_time_cmp final : comparator {
  explicit mod_time_cmp(common::fs_proxy *const aFs) : fs{aFs} {
    // force 2 lines
  }

  // ret = -1 -> lhs < rhs; ret = 0 -> equal; ret = 1 -> rhs < lhs
  [[nodiscard]] int compare(build_request const &lhs,
                            build_request const &rhs) const override {
    auto *const lhs_ft{dynamic_cast<common::file_target *>(lhs.tgt)};
    auto *const rhs_ft{dynamic_cast<common::file_target *>(rhs.tgt)};

    if (lhs_ft == nullptr || rhs_ft == nullptr) {
      // fallback ...:
      return name_cmp<asc>{}.compare(lhs, rhs);
    }

    auto const &lhs_path{lhs_ft->get_resolved_path()};
    auto const &rhs_path{rhs_ft->get_resolved_path()};

    auto const lhs_ex{fs->file_exists(lhs_path)};
    auto const rhs_ex{fs->file_exists(rhs_path)};

    if (lhs_ex && rhs_ex) {
      auto const lhs_mod_time{fs->file_last_mod_time(lhs_path)};
      auto const rhs_mod_time{fs->file_last_mod_time(rhs_path)};

      return signum(asc ? lhs_mod_time - rhs_mod_time
                        : rhs_mod_time - lhs_mod_time);
    }
    // fallbacks ...:
    else if (lhs_ex) {
      return asc ? -1 : 1;
    } else if (rhs_ex) {
      return asc ? 1 : -1;
    } else {
      return asc ? lhs_path < rhs_path : rhs_path < lhs_path;
    }
  }

  // must be set; not owned
  common::fs_proxy *const fs;
};

} // namespace

std::unique_ptr<comparator>
make_comparator_chain(std::vector<std::string_view> const &comparator_names,
                      common::fs_proxy *const fs) {
  if (comparator_names.empty()) {
    return std::make_unique<name_cmp<true>>();
  }

  std::unique_ptr<comparator> head{nullptr};
  comparator *tail{nullptr};

  for (auto const name : comparator_names) {
    std::unique_ptr<comparator> cmp;

    if (name == sort_by::name_asc) {
      cmp = std::make_unique<name_cmp<true>>();
    } else if (name == sort_by::name_desc) {
      cmp = std::make_unique<name_cmp<false>>();
    } else if (name == sort_by::mod_time_asc) {
      cmp = std::make_unique<mod_time_cmp<true>>(fs);
    } else if (name == sort_by::mod_time_desc) {
      cmp = std::make_unique<mod_time_cmp<false>>(fs);
    } else {
      throw std::runtime_error{"Unknown comparator name '" + std::string{name} +
                               "'"};
    }

    if (head == nullptr) {
      head = std::move(cmp);
      tail = head.get();
    } else {
      tail->next = std::move(cmp);
      tail = tail->next.get();
    }
  }

  return head;
}

} // namespace build_cxx::driver
