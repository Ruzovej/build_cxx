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

#include "build_cxx/driver/build_request_comparator.hxx"

#include <build_cxx/common/file_target.hxx>

namespace build_cxx::driver {

namespace {

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

  mutable comparator const *next{nullptr};

protected:
  // ret = -1 -> lhs < rhs; ret = 0 -> equal; ret = 1 -> rhs < lhs
  [[nodiscard]] virtual int compare(build_request const &lhs,
                                    build_request const &rhs) const = 0;
};

template <bool asc> struct name_cmp final : comparator {
  // ret = -1 -> lhs < rhs; ret = 0 -> equal; ret = 1 -> rhs < lhs
  [[nodiscard]] int compare(build_request const &lhs,
                            build_request const &rhs) const override {
    std::string_view const lhs_name{lhs.tgt->resolved_name};
    std::string_view const rhs_name{rhs.tgt->resolved_name};

    return asc ? lhs_name.compare(rhs_name) : rhs_name.compare(lhs_name);
  }
};

[[nodiscard]] comparator const *get_name_cmp(bool const asc) {
  static name_cmp<true> const name_cmp_asc;
  static name_cmp<false> const name_cmp_desc;

  return asc ? static_cast<comparator const *>(&name_cmp_asc)
             : static_cast<comparator const *>(&name_cmp_desc);
}

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

comparator const *first_comparator{nullptr};
comparator const *last_comparator{nullptr};

} // namespace

bool build_request_comparator::operator()(build_request const &lhs,
                                          build_request const &rhs) const {
  // sensible default?
  auto *const cmp{first_comparator ? first_comparator : get_name_cmp(true)};

  // https://en.cppreference.com/w/cpp/container/priority_queue.html ... to
  // prevent confusion: the lowest value means being processed last by the
  // priority queue, so we need to invert the result of comparison:
  return (*cmp)(rhs, lhs);
}

void make_comparator_chain(
    std::vector<std::string_view> const &comparator_names,
    common::fs_proxy *const fs) {
  for (auto const cmp_name : comparator_names) {
    comparator const *candidate{nullptr};

    if (cmp_name == sort_by::name_asc) {
      candidate = get_name_cmp(true);
    } else if (cmp_name == sort_by::name_desc) {
      candidate = get_name_cmp(false);
    } else if (cmp_name == sort_by::mod_time_asc) {
      candidate = get_mod_time_cmp(true, fs);
    } else if (cmp_name == sort_by::mod_time_desc) {
      candidate = get_mod_time_cmp(false, fs);
    }

    if (candidate == nullptr) {
      throw std::runtime_error{"Unknown (or unimplemented) comparator name '" +
                               std::string{cmp_name} + "'"};
    }

    if (candidate->next != nullptr) {
      throw std::runtime_error{"Comparator '" + std::string{cmp_name} +
                               "' is already in the chain"};
    }

    if (first_comparator == nullptr) {
      first_comparator = candidate;
      last_comparator = candidate;
    } else {
      last_comparator->next = candidate;
      last_comparator = candidate;
    }
  }
}

} // namespace build_cxx::driver
