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
#include "build_cxx/driver/build_request_priority_queue.hxx"

#include <doctest/doctest.h>

#include "build_cxx/common/abstract_target.hxx"
#include "build_cxx/driver/processed_targets.hxx"
#include "build_cxx/driver/scheduler.hxx"
#include "build_cxx/test_helpers/mock_fs.hxx"
#include "build_cxx/test_helpers/mock_project.hxx"

namespace build_cxx {
namespace {

driver::build_request_comparators_chain::comparators_chain
make_comparators_chain(std::vector<std::string_view> const &input) {
  return driver::build_request_comparators_chain::make_comparators_chain(input);
};

[[nodiscard]] driver::scheduler create_sched(
    test_helpers::mock_fs *const fake_fs,
    driver::build_request_comparators_chain::comparators_chain &&comps) {
  return driver::scheduler{fake_fs, std::move(comps), 1, false};
};

[[nodiscard]] driver::build_request_priority_queue make_prio_queue(
    test_helpers::mock_fs *const fake_fs,
    driver::build_request_comparators_chain::comparators_chain const &comps) {
  return driver::build_request_priority_queue{
      driver::build_request_comparators_chain{fake_fs, comps}};
}

void require_top_and_pop(driver::build_request_priority_queue &pq,
                         common::abstract_target const *const expected) {
  // set this to `true` while debugging failure in this function, to `false` for
  // performance:
  static bool constexpr more_readable_doctest_output_on_failure{true};
  if constexpr (more_readable_doctest_output_on_failure) {
    REQUIRE_EQ(pq.top().tgt->resolved_name, expected->resolved_name);
  } else {
    REQUIRE_EQ(pq.top().tgt, expected);
  }
  REQUIRE_NOTHROW(pq.pop());
}

TEST_CASE("driver::build_request_comparators_chain") {
  SUBCASE("...::make_comparators_chain") {
    driver::build_request_comparators_chain::comparators_chain comps;

    SUBCASE("valid inputs") {
      SUBCASE("empty input") {
        REQUIRE_NOTHROW(comps = make_comparators_chain({}));
        REQUIRE(comps.empty());
      }

      SUBCASE("single input") {
        REQUIRE_NOTHROW(
            comps = make_comparators_chain({driver::sort_by::name_asc}));
        REQUIRE_EQ(comps.size(), 1);
      }

      SUBCASE("all valid ones") {
        // "random" combination of them ...
        REQUIRE_NOTHROW(
            comps = make_comparators_chain({driver::sort_by::name_asc,
                                            driver::sort_by::doesnt_exist,
                                            driver::sort_by::mod_time_desc}));
        REQUIRE_EQ(comps.size(), 3);
      }
    }

    SUBCASE("invalid inputs") {
      SUBCASE("only invalid") {
        REQUIRE_THROWS(comps = make_comparators_chain({"some_invalid_name"}));
        REQUIRE_THROWS(comps = make_comparators_chain(
                           {"some_invalid_name", "another_one"}));
      }

      SUBCASE("some valid, at least one invalid") {
        REQUIRE_THROWS(comps = make_comparators_chain(
                           {driver::sort_by::name_asc, "some_invalid_name"}));
        REQUIRE_THROWS(comps = make_comparators_chain(
                           {"some_invalid_name", driver::sort_by::name_desc}));
      }

      SUBCASE("duplicated valid ones") {
        REQUIRE_THROWS(comps =
                           make_comparators_chain({driver::sort_by::name_asc,
                                                   driver::sort_by::name_asc}));
      }

      SUBCASE("mutualy exclusive valid ones") {
        REQUIRE_THROWS(
            comps = make_comparators_chain(
                {driver::sort_by::name_asc, driver::sort_by::name_desc}));
      }
    }
  }

  SUBCASE("determined order") {
    // TODO this should utilize only one thread: mutex, thread spawning, etc.
    // should be avoided

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

    // alias targets (TODO use/test them):

    auto *const aaa_af_1{test_project_aaa.add_target_alias(
        fake_filename_aaa, false, "aaa_1.c", {"src/aaa_1.c"})};

    auto *const aaa_af_2{test_project_aaa.add_target_alias(
        fake_filename_aaa, false, "aaa_2.c", {"src/aaa_2.c"})};

    auto *const aaa_af_3{test_project_aaa.add_target_alias(
        fake_filename_aaa, false, "aaa_3.c", {"src/aaa_3.c"})};

    auto *const aaa_ap_1{test_project_aaa.add_target_alias(
        fake_filename_aaa, false, "aaa_p_1", {"pt_1"})};

    auto *const aaa_ap_2{test_project_aaa.add_target_alias(
        fake_filename_aaa, false, "aaa_p_2", {"pt_1"})};

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

    // alias targets (TODO use/test them):

    auto *const zzz_af_1{test_project_zzz.add_target_alias(
        fake_filename_zzz, false, "zzz_1.c", {"src/zzz_1.c"})};

    auto *const zzz_af_2{test_project_zzz.add_target_alias(
        fake_filename_zzz, false, "zzz_2.c", {"src/zzz_2.c"})};

    auto *const zzz_af_3{test_project_zzz.add_target_alias(
        fake_filename_zzz, false, "zzz_3.c", {"src/zzz_3.c"})};

    auto *const zzz_ap_1{test_project_zzz.add_target_alias(
        fake_filename_zzz, false, "zzz_p_1", {"pt_1"})};

    auto *const zzz_ap_2{test_project_zzz.add_target_alias(
        fake_filename_zzz, false, "zzz_p_2", {"pt_1"})};

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

    auto sched{create_sched(&fake_fs, {})};

    driver::processed_targets driver_pt{sched};

    REQUIRE_NOTHROW(driver_pt.process_project(&test_project_aaa));
    REQUIRE_NOTHROW(driver_pt.process_project(&test_project_zzz));

    bool all_resolved{false};
    REQUIRE_NOTHROW(all_resolved = driver_pt.resolve_deps_for_all());
    REQUIRE(all_resolved);

    fake_fs.touch(zzz_f_3->get_resolved_path());
    fake_fs.touch(zzz_f_1->get_resolved_path());
    // so it "doesn't exist":
    // fake_fs.touch(zzz_f_2->get_resolved_path());
    fake_fs.touch(aaa_f_3->get_resolved_path());
    fake_fs.touch(aaa_f_1->get_resolved_path());
    // so it "doesn't exist":
    // fake_fs.touch(aaa_f_2->get_resolved_path());

    SUBCASE("ascending by resolved name") {
      // arrange
      driver::build_request_comparators_chain::comparators_chain comps;

      SUBCASE("default") {
        // force 2 lines
        comps = make_comparators_chain({});
      }

      SUBCASE("explicit") {
        comps = make_comparators_chain({driver::sort_by::name_asc});
      }

      auto pq{make_prio_queue(&fake_fs, comps)};

      // act (push them in "random" order)
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_3, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_3, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_pt_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_pt_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_pt_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_pt_1, nullptr}));

      // assert
      REQUIRE_EQ(pq.size(), 10);

      require_top_and_pop(pq, aaa_f_1);

      require_top_and_pop(pq, aaa_f_2);

      require_top_and_pop(pq, aaa_f_3);

      require_top_and_pop(pq, zzz_f_1);

      require_top_and_pop(pq, zzz_f_2);

      require_top_and_pop(pq, zzz_f_3);

      require_top_and_pop(pq, aaa_pt_1);

      require_top_and_pop(pq, aaa_pt_2);

      require_top_and_pop(pq, zzz_pt_1);

      require_top_and_pop(pq, zzz_pt_2);
    }

    SUBCASE("descending by resolved name") {
      // arrange
      auto comps{make_comparators_chain({driver::sort_by::name_desc})};

      auto pq{make_prio_queue(&fake_fs, comps)};

      // act (push them in "random" order)
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_3, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_3, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_pt_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_pt_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_pt_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_pt_1, nullptr}));

      // assert
      REQUIRE_EQ(pq.size(), 10);

      require_top_and_pop(pq, zzz_pt_2);

      require_top_and_pop(pq, zzz_pt_1);

      require_top_and_pop(pq, aaa_pt_2);

      require_top_and_pop(pq, aaa_pt_1);

      require_top_and_pop(pq, zzz_f_3);

      require_top_and_pop(pq, zzz_f_2);

      require_top_and_pop(pq, zzz_f_1);

      require_top_and_pop(pq, aaa_f_3);

      require_top_and_pop(pq, aaa_f_2);

      require_top_and_pop(pq, aaa_f_1);
    }

    SUBCASE("prefer existing files, then ascending by resolved name") {
      // arrange
      driver::build_request_comparators_chain::comparators_chain comps;

      SUBCASE("default") {
        // force 2 lines
        comps = make_comparators_chain({driver::sort_by::exists});
      }

      SUBCASE("explicit") {
        comps = make_comparators_chain(
            {driver::sort_by::exists, driver::sort_by::name_asc});
      }

      auto pq{make_prio_queue(&fake_fs, comps)};

      // act (push them in "random" order)
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_3, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_3, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_pt_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_pt_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_pt_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_pt_1, nullptr}));

      // assert
      REQUIRE_EQ(pq.size(), 10);

      require_top_and_pop(pq, aaa_f_1);

      require_top_and_pop(pq, aaa_f_3);

      require_top_and_pop(pq, zzz_f_1);

      require_top_and_pop(pq, zzz_f_3);

      require_top_and_pop(pq, aaa_f_2);

      require_top_and_pop(pq, zzz_f_2);

      require_top_and_pop(pq, aaa_pt_1);

      require_top_and_pop(pq, aaa_pt_2);

      require_top_and_pop(pq, zzz_pt_1);

      require_top_and_pop(pq, zzz_pt_2);
    }

    SUBCASE("prefer non-existing files, then ascending by resolved name") {
      // arrange
      driver::build_request_comparators_chain::comparators_chain comps;

      SUBCASE("default") {
        // force 2 lines
        comps = make_comparators_chain({driver::sort_by::doesnt_exist});
      }

      SUBCASE("explicit") {
        comps = make_comparators_chain(
            {driver::sort_by::doesnt_exist, driver::sort_by::name_asc});
      }

      auto pq{make_prio_queue(&fake_fs, comps)};

      // act (push them in "random" order)
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_3, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_3, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_pt_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_pt_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_f_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{aaa_pt_2, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_f_1, nullptr}));
      REQUIRE_NOTHROW(pq.emplace(driver::build_request{zzz_pt_1, nullptr}));

      // assert
      REQUIRE_EQ(pq.size(), 10);

      require_top_and_pop(pq, aaa_f_2);

      require_top_and_pop(pq, zzz_f_2);

      require_top_and_pop(pq, aaa_pt_1);

      require_top_and_pop(pq, aaa_pt_2);

      require_top_and_pop(pq, zzz_pt_1);

      require_top_and_pop(pq, zzz_pt_2);

      require_top_and_pop(pq, aaa_f_1);

      require_top_and_pop(pq, aaa_f_3);

      require_top_and_pop(pq, zzz_f_1);

      require_top_and_pop(pq, zzz_f_3);
    }

    // TODO ...
  }
}

} // namespace
} // namespace build_cxx
