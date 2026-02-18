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

#include <doctest/doctest.h>

#include "build_cxx/driver/processed_targets.hxx"
#include "build_cxx/driver/scheduler.hxx"
#include "build_cxx/test_helpers/mock_fs.hxx"
#include "build_cxx/test_helpers/mock_project.hxx"

namespace build_cxx {
namespace {

[[nodiscard]] driver::scheduler create_sched(
    test_helpers::mock_fs *const fake_fs,
    driver::build_request_comparators_chain::comparators_chain &&comps) {
  return driver::scheduler{fake_fs, std::move(comps), 1, false};
};

TEST_CASE("driver::build_request_comparators_chain") {
  std::mutex mtx;
  test_helpers::built_targets_t built_targets;
  test_helpers::mock_fs fake_fs;

  // generally, we don't care about nontrivial dependency graph, etc. - just
  // having everything independent is enough to test this feature, all targets
  // just need to get their names, etc. resolved

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // first project, aaa:

  static std::string_view constexpr fake_filename_aaa{
      "/fake/dir/aaa/build.cxx"};

  test_helpers::mock_project test_project_aaa{
      &mtx, &built_targets, &fake_fs, "aaa", "0.13.57", fake_filename_aaa};

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // alias targets:

  // TODO

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // file targets:

  auto *const aaa_f_1{test_project_aaa.add_mock_file_target(
      fake_filename_aaa, false, "src/aaa_1.c", true, {})};

  auto *const aaa_f_2{test_project_aaa.add_mock_file_target(
      fake_filename_aaa, false, "src/aaa_2.c", true, {})};

  auto *const aaa_f_3{test_project_aaa.add_mock_file_target(
      fake_filename_aaa, false, "src/aaa_3.c", true, {})};

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // phony targets:

  auto *const aaa_pt_1{test_project_aaa.add_mock_phony_target(
      fake_filename_aaa, true, "pt_1", {})};

  auto *const aaa_pt_2{test_project_aaa.add_mock_phony_target(
      fake_filename_aaa, true, "pt_2", {})};

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // second project, zzz:

  static std::string_view constexpr fake_filename_zzz{
      "/fake/dir/zzz/build.cxx"};

  test_helpers::mock_project test_project_zzz{
      &mtx, &built_targets, &fake_fs, "zzz", "1.3.107", fake_filename_zzz};

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // alias targets:

  // TODO

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // file targets:

  auto *const zzz_f_1{test_project_zzz.add_mock_file_target(
      fake_filename_zzz, false, "src/zzz_1.c", true, {})};

  auto *const zzz_f_2{test_project_zzz.add_mock_file_target(
      fake_filename_zzz, false, "src/zzz_2.c", true, {})};

  auto *const zzz_f_3{test_project_zzz.add_mock_file_target(
      fake_filename_zzz, false, "src/zzz_3.c", true, {})};

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // phony targets:

  auto *const zzz_pt_1{test_project_zzz.add_mock_phony_target(
      fake_filename_zzz, true, "pt_1", {})};

  auto *const zzz_pt_2{test_project_zzz.add_mock_phony_target(
      fake_filename_zzz, true, "pt_2", {})};

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  auto const pt_process = [&](driver::processed_targets &driver_pt) {
    REQUIRE_NOTHROW(driver_pt.process_project(&test_project_aaa));
    REQUIRE_NOTHROW(driver_pt.process_project(&test_project_zzz));

    bool all_resolved{false};
    REQUIRE_NOTHROW(all_resolved = driver_pt.resolve_deps_for_all());
    REQUIRE(all_resolved);
  };

  SUBCASE("default chain") {
    auto sched{create_sched(&fake_fs, {})};

    driver::processed_targets driver_pt{sched};

    pt_process(driver_pt);

    // ... TODO ...
  }
}

} // namespace
} // namespace build_cxx
