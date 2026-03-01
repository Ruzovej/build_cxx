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

#include <array>
#include <memory>
#include <queue>
#include <string_view>

#include <build_cxx/common/fs_proxy.hxx>
#include <build_cxx/common/macros.h>

#include "build_cxx/driver/build_request.hxx"

namespace build_cxx::driver {

namespace sort_by {

// Sort build requests by their resolved target name in ascending
// (lexicographic) order. Targets whose names compare smaller are processed
// first.
//
// Mutually exclusive with `name_desc` — specifying both in the same chain will
// throw. Duplicating this entry also throws.
//
// This is also the implicit fallback comparator: when every explicitly
// requested comparator in the chain returns equivalence, the queue falls back
// to ascending-name ordering. Consequently, requesting `name_asc` as the last
// entry in a chain is valid but redundant.
static std::string_view constexpr name_asc{"name_asc"};

// Sort build requests by their resolved target name in descending
// (reverse-lexicographic) order. Targets whose names compare larger are
// processed first.
//
// Mutually exclusive with `name_asc` — specifying both in the same chain will
// throw.
static std::string_view constexpr name_desc{"name_desc"};

// Sort build requests by the modification time of their newest dependency,
// ascending (oldest first). Targets backed by files with a known modification
// time are ordered from oldest to newest; targets that "certainly need update"
// (phony targets, non-existent files, etc.) are treated as "newer than any real
// timestamp" and therefore appear last.
//
// When two targets share the same effective modification-time bucket the
// comparator returns equivalence (0) and the next comparator in the chain (or
// the implicit ascending-name fallback) breaks the tie.
//
// Mutually exclusive with `mod_time_desc` — specifying both in the same chain
// will throw.
static std::string_view constexpr mod_time_asc{"mod_time_asc"};

// Sort build requests by the modification time of their newest dependency,
// descending (newest first). Targets that "certainly need update" (phony
// targets, non-existent files, etc.) are treated as the "newest" and therefore
// appear first; among real files the one with the most recent modification
// time comes first.
//
// Mutually exclusive with `mod_time_asc` — specifying both in the same chain
// will throw.
static std::string_view constexpr mod_time_desc{"mod_time_desc"};

// Prefer targets whose underlying file already exists on the filesystem.
// Existing file targets are processed before non-existent ones (and before
// phony targets, which are always treated as non-existent). When both targets
// have the same existence status the comparator returns equivalence and the
// next comparator in the chain decides. If no further comparator is given, the
// implicit ascending-name fallback is used.
//
// Only meaningful for file_target instances — other target types (phony, alias,
// ...) are treated as non-existent.
//
// Mutually exclusive with `doesnt_exist` — specifying both in the same chain
// will throw.
static std::string_view constexpr exists{"exists"};

// Prefer targets whose underlying file does not exist on the filesystem.
// Non-existent file targets (and phony targets) are processed before existing
// ones. When both targets have the same existence status the comparator returns
// equivalence and the next comparator in the chain decides. If no further
// comparator is given, the implicit ascending-name fallback is used.
//
// Only meaningful for file_target instances — other target types (phony, alias,
// ...) are treated as non-existent.
//
// Mutually exclusive with `exists` — specifying both in the same chain will
// throw.
static std::string_view constexpr doesnt_exist{"doesnt_exist"};

// TODO ...:
// static std::string_view constexpr prev_fail{"prev_fail"};
// static std::string_view constexpr prev_fail{"prefer_file_target"};

} // namespace sort_by

// TODO ... EXPORT or HIDE?!
struct BUILD_CXX_DLL_EXPORT build_request_comparators_chain {
  // return:
  // - `-1` if `lhs` has higher priority (should be processed sooner) than `rhs`
  // - `1` if `rhs` has higher priority than `lhs`
  // - `0` if they are equivalent (or equal?!)
  using comparator_fn = int(build_request const &lhs, build_request const &rhs,
                            common::fs_proxy *const fs);

  // not owning any pointer:
  using comparators_chain = std::array<comparator_fn *, 3>;

  explicit build_request_comparators_chain(
      common::fs_proxy *const aFs, comparators_chain const &aComps) noexcept;

  // true if `lhs` has higher priority (should be processed sooner) than `rhs`
  [[nodiscard]] bool operator()(build_request const &lhs,
                                build_request const &rhs) const;

  [[nodiscard]] static comparators_chain
  make_comparators_chain(std::vector<std::string_view> const &comparator_names);

private:
  common::fs_proxy *fs;
  comparator_fn *const *comps;
  std::size_t n_comps;
};

} // namespace build_cxx::driver
