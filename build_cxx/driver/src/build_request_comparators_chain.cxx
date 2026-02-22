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
// ret = -1 -> lhs < rhs; ret = 0 -> equal; ret = 1 -> rhs < lhs
[[nodiscard]] int name_compare(build_request const &lhs,
                               build_request const &rhs,
                               common::fs_proxy *const fs) {
  static_cast<void>(fs); // unused

  std::string_view const lhs_name{lhs.tgt->resolved_name};
  std::string_view const rhs_name{rhs.tgt->resolved_name};

  return asc ? lhs_name.compare(rhs_name) : rhs_name.compare(lhs_name);
}

template <bool asc = true>
[[nodiscard]] int fallback_compare(build_request const &lhs,
                                   build_request const &rhs,
                                   common::fs_proxy *const fs) {
  return name_compare<asc>(lhs, rhs, fs);
}

[[nodiscard]] build_request_comparators_chain::comparator_fn *
get_name_cmp(bool const asc) {
  return asc ? static_cast<build_request_comparators_chain::comparator_fn *>(
                   &name_compare<true>)
             : static_cast<build_request_comparators_chain::comparator_fn *>(
                   &name_compare<false>);
}

template <bool asc>
[[nodiscard]] int simple_file_exists_comparison_res(bool const lhs_ex,
                                                    bool const rhs_ex) {
  if (lhs_ex && rhs_ex) {
    return 0; // equivalent
  }
  // fallbacks ...:
  else if (lhs_ex) {
    // rhs doesn't exist ... lhs has precedence
    return asc ? -1 : 1;
  } else if (rhs_ex) {
    // lhs doesn't exist ... rhs has precedence
    return asc ? 1 : -1;
  } else {
    // none exists ... equivalent:
    return 0;
  }
}

template <bool asc>
// ret = -1 -> lhs < rhs; ret = 0 -> equal; ret = 1 -> rhs < lhs
[[nodiscard]] int file_exists_compare(build_request const &lhs,
                                      build_request const &rhs,
                                      common::fs_proxy *const fs) {
  auto *const lhs_ft{dynamic_cast<common::file_target *>(lhs.tgt)};
  auto *const rhs_ft{dynamic_cast<common::file_target *>(rhs.tgt)};

  if (lhs_ft == nullptr || rhs_ft == nullptr) {
    // IMHO if one of them really is a file target, there's no need to further
    // check for its existence compared to some other type:
    return simple_file_exists_comparison_res<asc>(lhs_ft != nullptr,
                                                  rhs_ft != nullptr);
  }

  auto const &lhs_path{lhs_ft->get_resolved_path()};
  auto const lhs_ex{fs->file_exists(lhs_path)};

  auto const &rhs_path{rhs_ft->get_resolved_path()};
  auto const rhs_ex{fs->file_exists(rhs_path)};

  return simple_file_exists_comparison_res<asc>(lhs_ex, rhs_ex);
}

[[nodiscard]] build_request_comparators_chain::comparator_fn *
get_file_exists_cmp(bool const asc) {
  return asc ? static_cast<build_request_comparators_chain::comparator_fn *>(
                   &file_exists_compare<true>)
             : static_cast<build_request_comparators_chain::comparator_fn *>(
                   &file_exists_compare<false>);
}

template <typename T> [[nodiscard]] static T signum(T val) {
  if (val < 0) {
    return -1;
  } else if (val > 0) {
    return 1;
  } else {
    return 0;
  }
}

template <bool asc>
// ret = -1 -> lhs < rhs; ret = 0 -> equal; ret = 1 -> rhs < lhs
[[nodiscard]] int mod_time_compare(build_request const &lhs,
                                   build_request const &rhs,
                                   common::fs_proxy *const fs) {
  auto *const lhs_ft{dynamic_cast<common::file_target *>(lhs.tgt)};
  auto *const rhs_ft{dynamic_cast<common::file_target *>(rhs.tgt)};

  if (lhs_ft == nullptr || rhs_ft == nullptr) {
    return fallback_compare<asc>(lhs, rhs, fs);
  }

  auto const &lhs_path{lhs_ft->get_resolved_path()};
  auto const lhs_ex{fs->file_exists(lhs_path)};

  auto const &rhs_path{rhs_ft->get_resolved_path()};
  auto const rhs_ex{fs->file_exists(rhs_path)};

  if (lhs_ex && rhs_ex) {
    auto const lhs_mod_time{fs->file_last_mod_time(lhs_path)};
    auto const rhs_mod_time{fs->file_last_mod_time(rhs_path)};

    return signum(asc ? lhs_mod_time - rhs_mod_time
                      : rhs_mod_time - lhs_mod_time);
  }
  // fallbacks ...:
  else if (lhs_ex) {
    // rhs doesn't exist ... it has precedence
    return asc ? 1 : -1;
  } else if (rhs_ex) {
    // lhs doesn't exist ... it has precedence
    return asc ? -1 : 1;
  } else {
    // none exists ... fallback to name comparison:
    return fallback_compare<asc>(lhs, rhs, fs);
  }
}

[[nodiscard]] build_request_comparators_chain::comparator_fn *
get_mod_time_cmp(bool const asc) {
  return asc ? static_cast<build_request_comparators_chain::comparator_fn *>(
                   &mod_time_compare<true>)
             : static_cast<build_request_comparators_chain::comparator_fn *>(
                   &mod_time_compare<false>);
}

} // namespace

build_request_comparators_chain::build_request_comparators_chain(
    common::fs_proxy *const aFs, comparators_chain const &aComps) noexcept
    : fs{aFs}, comps{aComps.data()}, n_comps{aComps.size()} {
  // force 2 lines
}

bool build_request_comparators_chain::operator()(
    build_request const &lhs, build_request const &rhs) const {
  for (std::size_t i{0}; (i < n_comps) && (comps[i] != nullptr); ++i) {
    auto *const cmp{comps[i]};

    // https://en.cppreference.com/w/cpp/container/priority_queue.html ... to
    // prevent confusion: the lowest value means being processed last by the
    // priority queue, so we need to invert the order of comparison(s) here:
    auto const cmp_res{cmp(rhs, lhs, fs)};

    if (cmp_res < 0) {
      return true;
    } else if (0 < cmp_res) {
      return false;
    } else { // cmp_res == 0 -> try the next comparator in the chain
      continue;
    }
  }

  // they are equivalent (or equal?!); internally, this is "sort by name,
  // ascending", so it be slightly inefficient if the last `comps` member is
  // exactly this
  return fallback_compare(rhs, lhs, fs) <= -1;
}

build_request_comparators_chain::comparators_chain
build_request_comparators_chain::make_comparators_chain(
    std::vector<std::string_view> const &comparator_names) {
  comparators_chain res;
  res.reserve(comparator_names.size());

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
    } else if (cmp_name == sort_by::exists) {
      res.push_back(get_file_exists_cmp(true));
      used_or_blocked.insert({sort_by::exists, sort_by::doesnt_exist});
    } else if (cmp_name == sort_by::doesnt_exist) {
      res.push_back(get_file_exists_cmp(false));
      used_or_blocked.insert({sort_by::exists, sort_by::doesnt_exist});
    } else if (cmp_name == sort_by::mod_time_asc) {
      res.push_back(get_mod_time_cmp(true));
      used_or_blocked.insert({sort_by::mod_time_asc, sort_by::mod_time_desc});
    } else if (cmp_name == sort_by::mod_time_desc) {
      res.push_back(get_mod_time_cmp(false));
      used_or_blocked.insert({sort_by::mod_time_asc, sort_by::mod_time_desc});
    } else {
      throw std::runtime_error{"Unknown (or unimplemented) comparator '" +
                               std::string{cmp_name} + '\''};
    }
  }

  return res;
}

} // namespace build_cxx::driver
