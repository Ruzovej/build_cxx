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

#include <optional>
#include <unordered_set>

#include <build_cxx/common/file_target.hxx>

#include "build_cxx/utility/unreachable.hxx"

namespace build_cxx::driver {

namespace {

template <bool asc>
[[nodiscard]] int name_compare(build_request const &lhs,
                               build_request const &rhs,
                               common::fs_proxy *const fs) {
  static_cast<void>(fs); // unused

  std::string_view const lhs_name{lhs.tgt->resolved_name};
  std::string_view const rhs_name{rhs.tgt->resolved_name};

  return asc ? lhs_name.compare(rhs_name) : rhs_name.compare(lhs_name);
}

[[nodiscard]] int fallback_compare(build_request const &lhs,
                                   build_request const &rhs,
                                   common::fs_proxy *const fs) {
  return name_compare<true>(lhs, rhs, fs);
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
  if (lhs_ex == rhs_ex) {
    // both or none exist ... equivalent:
    return 0;
  } else if (lhs_ex) {
    // rhs doesn't exist ... lhs has precedence (when ascending)
    return asc ? -1 : 1;
  } else if (rhs_ex) {
    // lhs doesn't exist ... rhs has precedence (when ascending)
    return asc ? 1 : -1;
  } else {
    utility::unreachable(); // without that, there was compiler warning, etc.
  }
}

[[nodiscard]] std::optional<bool>
get_target_exists(build_request const &brq, common::fs_proxy *const fs) {
  auto *const file_tgt{dynamic_cast<common::file_target const *>(brq.tgt)};
  if (file_tgt != nullptr) {
    return fs->file_exists(file_tgt->get_resolved_path());
  }

  // TODO special treatment for alias targets, etc.?

  return std::nullopt;
}

template <bool asc>
[[nodiscard]] int file_exists_compare(build_request const &lhs,
                                      build_request const &rhs,
                                      common::fs_proxy *const fs) {
  return simple_file_exists_comparison_res<asc>(
      get_target_exists(lhs, fs).value_or(false),
      get_target_exists(rhs, fs).value_or(false));
}

[[nodiscard]] build_request_comparators_chain::comparator_fn *
get_file_exists_cmp(bool const asc) {
  return asc ? static_cast<build_request_comparators_chain::comparator_fn *>(
                   &file_exists_compare<true>)
             : static_cast<build_request_comparators_chain::comparator_fn *>(
                   &file_exists_compare<false>);
}

template <bool asc>
[[nodiscard]] int mod_time_compare(build_request const &lhs,
                                   build_request const &rhs,
                                   common::fs_proxy *const fs) {
  static_cast<void>(fs); // unused

  // handle non-existent files and phony targets first:
  const bool lhs_certainly_needs_update{
      lhs.newest_dep_status.certainly_needs_update()};
  const bool rhs_certainly_needs_update{
      rhs.newest_dep_status.certainly_needs_update()};

  if (lhs_certainly_needs_update && rhs_certainly_needs_update) {
    // both certainly need update ... equivalent:
    return 0;
  } else if (lhs_certainly_needs_update) {
    // `rhs` doesn't certainly need update ... it's perceived as "older" than
    // `lhs`
    return asc ? 1 : -1;
  } else if (rhs_certainly_needs_update) {
    // `lhs` doesn't certainly need update ... it's perceived as "older" than
    // `rhs`
    return asc ? -1 : 1;
  }

  // both are files, and have "valid" mod. times:
  const bool lhs_needs_update{
      lhs.newest_dep_status.needs_update_compared_to(rhs.newest_dep_status)};
  const bool rhs_needs_update{
      rhs.newest_dep_status.needs_update_compared_to(lhs.newest_dep_status)};

  if (lhs_needs_update == rhs_needs_update) {
    // equivalent ...
    return 0;
  } else if (lhs_needs_update) {
    // `lhs` is newer than `rhs`
    return asc ? -1 : 1;
  } else if (rhs_needs_update) {
    // `rhs` is newer than `lhs`
    return asc ? 1 : -1;
  }

  utility::unreachable(); // prevents compiler warning, etc.
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
    : fs{aFs}, comps{&aComps} {
  // force 2 lines
}

bool build_request_comparators_chain::operator()(
    build_request const &lhs, build_request const &rhs) const {
  for (std::size_t i{0}; i < comps->size(); ++i) {
    auto *const cmp_fn{comps->at(i)};
    if (cmp_fn == nullptr) {
      // no more comparators in the chain; they are equivalent (or equal?!)
      break;
    }

    // https://en.cppreference.com/w/cpp/container/priority_queue.html ... the
    // lowest value means being processed last by the priority queue
    auto const cmp_res{cmp_fn(lhs, rhs, fs)};

    if (cmp_res < 0) {
      // `lhs` < `rhs` -> `lhs` has higher priority (should be processed sooner)
      return false;
    } else if (0 < cmp_res) {
      // `rhs` < `lhs` -> ...
      return true;
    } else {
      // `lhs` ~= `rhs` ... equivalent (or equal?!) -> try the next comparator
      // in the chain
      continue;
    }
  }

  // they are equivalent (or equal?!); internally, this is "sort by name,
  // ascending", so it can be slightly inefficient if the last `comps` member is
  // exactly this; but on the other hand, if two filenames "compare equal" ...
  // it's a problem by itself:
  return 0 < fallback_compare(lhs, rhs, fs);
}

build_request_comparators_chain::comparators_chain
build_request_comparators_chain::make_comparators_chain(
    std::vector<std::string_view> const &comparator_names) {
  comparators_chain res{};
  int idx{0};

  std::unordered_set<std::string_view> used_or_blocked;

  for (auto const cmp_name : comparator_names) {
    if (used_or_blocked.find(cmp_name) != used_or_blocked.cend()) {
      throw std::runtime_error{
          "Comparator '" + std::string{cmp_name} +
          "' is already in the chain or blocked by another comparator"};
    }

    if (res.size() <= idx) {
      // if this happens, good question is why it wasn't "caught"/detected by
      // the `used_or_blocked` related guardrails & checks above?!
      throw std::runtime_error{
          "Too many comparators in the input - maximum is " +
          std::to_string(res.size())};
    }

    // error prone ... TODO some better way?
    if (cmp_name == sort_by::name_asc) {
      res[idx] = get_name_cmp(true);
      used_or_blocked.insert({sort_by::name_asc, sort_by::name_desc});
    } else if (cmp_name == sort_by::name_desc) {
      res[idx] = get_name_cmp(false);
      used_or_blocked.insert({sort_by::name_asc, sort_by::name_desc});
    } else if (cmp_name == sort_by::exists) {
      res[idx] = get_file_exists_cmp(true);
      used_or_blocked.insert({sort_by::exists, sort_by::doesnt_exist});
    } else if (cmp_name == sort_by::doesnt_exist) {
      res[idx] = get_file_exists_cmp(false);
      used_or_blocked.insert({sort_by::exists, sort_by::doesnt_exist});
    } else if (cmp_name == sort_by::mod_time_asc) {
      res[idx] = get_mod_time_cmp(true);
      used_or_blocked.insert({sort_by::mod_time_asc, sort_by::mod_time_desc});
    } else if (cmp_name == sort_by::mod_time_desc) {
      res[idx] = get_mod_time_cmp(false);
      used_or_blocked.insert({sort_by::mod_time_asc, sort_by::mod_time_desc});
    } else {
      throw std::runtime_error{"Unknown (or unimplemented) comparator '" +
                               std::string{cmp_name} + '\''};
    }

    ++idx;
  }

  return res;
}

} // namespace build_cxx::driver
