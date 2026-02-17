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

#include "build_cxx/driver/build_request_comparators_chain.hxx"

#include <unordered_set>

#include <build_cxx/common/file_target.hxx>

namespace build_cxx::driver {

namespace {

template <bool asc>
struct name_cmp final : build_request_comparators_chain::comparator {
  // ret = -1 -> lhs < rhs; ret = 0 -> equal; ret = 1 -> rhs < lhs
  [[nodiscard]] int compare(build_request const &lhs,
                            build_request const &rhs) const override {
    std::string_view const lhs_name{lhs.tgt->resolved_name};
    std::string_view const rhs_name{rhs.tgt->resolved_name};

    return asc ? lhs_name.compare(rhs_name) : rhs_name.compare(lhs_name);
  }
};

[[nodiscard]] build_request_comparators_chain::comparator const *
get_name_cmp(bool const asc) {
  static name_cmp<true> const name_cmp_asc;
  static name_cmp<false> const name_cmp_desc;

  return asc ? static_cast<build_request_comparators_chain::comparator const *>(
                   &name_cmp_asc)
             : static_cast<build_request_comparators_chain::comparator const *>(
                   &name_cmp_desc);
}

} // namespace

build_request_comparators_chain::build_request_comparators_chain(
    comparators_chain const &aComps) noexcept
    : comps{aComps.data()}, n_comps{static_cast<int>(aComps.size())} {
  if (aComps.empty()) {
    // set default comparator; ugly gymnastics & shenanigans to avoid taking
    // address of an r-value ...:
    static auto default_cmp{get_name_cmp(true)};
    comps = &default_cmp;

    n_comps = 1;
  }
}

bool build_request_comparators_chain::operator()(
    build_request const &lhs, build_request const &rhs) const {
  for (int i{0}; i < n_comps; ++i) {
    auto *const cmp{comps[i]};

    // https://en.cppreference.com/w/cpp/container/priority_queue.html ... to
    // prevent confusion: the lowest value means being processed last by the
    // priority queue, so we need to invert the order of comparison(s) here:
    auto const cmp_res{cmp->compare(rhs, lhs)};

    if (cmp_res < 0) {
      return true;
    } else if (0 < cmp_res) {
      return false;
    }
    // else cmp_res == 0 -> try the next comparator in the chain
  }

  // equivalent (or equal?!)
  return false;
}

build_request_comparators_chain::comparators_chain
build_request_comparators_chain::make_comparators_chain(
    std::vector<std::string_view> const &comparator_names,
    common::fs_proxy *const fs) {
  comparators_chain res;

  std::unordered_set<std::string_view> used_or_blocked;

  for (auto const cmp_name : comparator_names) {
    if (used_or_blocked.find(cmp_name) != used_or_blocked.cend()) {
      throw std::runtime_error{
          "Comparator '" + std::string{cmp_name} +
          "' is already in the chain or blocked by another comparator"};
    }

    if (cmp_name == sort_by::name_asc) {
      res.push_back(get_name_cmp(true));
      used_or_blocked.insert({sort_by::name_asc, sort_by::name_desc});
    } else if (cmp_name == sort_by::name_desc) {
      res.push_back(get_name_cmp(false));
      used_or_blocked.insert({sort_by::name_asc, sort_by::name_desc});
      //} else if (cmp_name == sort_by::mod_time_asc) {
      //  res.push_back(get_mod_time_cmp(true, fs));
      //  used_or_blocked.insert({sort_by::mod_time_asc,
      //  sort_by::mod_time_desc});
      //} else if (cmp_name == sort_by::mod_time_desc) {
      //  res.push_back(get_mod_time_cmp(false, fs));
      //  used_or_blocked.insert({sort_by::mod_time_asc,
      //  sort_by::mod_time_desc});
    } else {
      throw std::runtime_error{"Unknown (or unimplemented) comparator '" +
                               std::string{cmp_name} + '\''};
    }
  }

  return res;
}
/*
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
      return name_cmp<asc>{}.compare(lhs, rhs);
    }
  }

  // must be set; not owned
  common::fs_proxy *const fs;
};

comparator const *get_mod_time_cmp(bool const asc, common::fs_proxy *const fs) {
  static mod_time_cmp<true> const mod_time_cmp_asc{fs};
  static mod_time_cmp<false> const mod_time_cmp_desc{fs};

  return asc ? static_cast<comparator const *>(&mod_time_cmp_asc)
             : static_cast<comparator const *>(&mod_time_cmp_desc);
}

} // namespace
*/
} // namespace build_cxx::driver
