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

#include "build_cxx/driver/processed_targets.hxx"

#include <doctest/doctest.h>

#include "build_cxx/client/core.hxx"
#include "build_cxx/common/location.hxx"
#include "build_cxx/driver/scheduler.hxx"
#include "build_cxx/test_helpers/mock_file_target.hxx"
#include "build_cxx/test_helpers/mock_fs.hxx"
#include "build_cxx/test_helpers/mock_phony_target.hxx"
#include "build_cxx/test_helpers/mock_project.hxx"

namespace build_cxx {
namespace {

static int constexpr n_workers{12};
// file scope ... so it won't get reinitialized for each test case
driver::scheduler sched{n_workers};

TEST_CASE("driver::processed_targets") {
  static std::string_view constexpr fake_root_file1{
      "/fake/dir/project1.root.cxx"};

  test_helpers::built_targets_t built_targets;
  test_helpers::mock_fs fake_fs;
  test_helpers::mock_project test_project1{&built_targets, &fake_fs, "dpttp1",
                                           "0.1.0", fake_root_file1};

  driver::processed_targets driver_pt{&sched};
  // reset it ...:
  driver_pt.limit_to_n_workers(n_workers);

  SUBCASE("num workers") {
    REQUIRE_THROWS(driver_pt.limit_to_n_workers(0));

    REQUIRE_EQ(sched.num_workers(), n_workers);
    REQUIRE_EQ(sched.num_utilized_workers(), n_workers);

    REQUIRE_NOTHROW(driver_pt.limit_to_n_workers(1));

    REQUIRE_EQ(sched.num_workers(), n_workers);
    REQUIRE_EQ(sched.num_utilized_workers(), 1);

    REQUIRE_NOTHROW(driver_pt.limit_to_n_workers(n_workers));

    REQUIRE_EQ(sched.num_workers(), n_workers);
    REQUIRE_EQ(sched.num_utilized_workers(), n_workers);

    REQUIRE_THROWS(driver_pt.limit_to_n_workers(n_workers + 1));

    REQUIRE_EQ(sched.num_workers(), n_workers);
    REQUIRE_EQ(sched.num_utilized_workers(), n_workers);
  }

  // TODO figure out better way how to run those test cases with varying number
  // of workers ... see the repetitive `SUBCASE("varying num. of workers") ...`
  // used (copy-pasted) below:

  SUBCASE("basics") {
    SUBCASE("empty project") {
      REQUIRE_NOTHROW(driver_pt.process_project(&test_project1));

      REQUIRE_EQ(driver_pt.projects_by_name.size(), 1);
      REQUIRE_EQ(driver_pt.projects_by_name.begin()->first, test_project1.name);
      REQUIRE_EQ(driver_pt.projects_by_name.begin()->second, &test_project1);
      REQUIRE_EQ(driver_pt.targets_by_project.size(), 0);
      REQUIRE_EQ(driver_pt.project_of_target.size(), 0);
      REQUIRE_EQ(driver_pt.targets_by_resolved_name.size(), 0);
      REQUIRE_EQ(driver_pt.intermediate_targets.size(), 0);
      // REQUIRE_EQ(pt.built_targets.size(), 0); // private ...
      REQUIRE_EQ(driver_pt.get_target_resolved_deps().size(), 0);
      // REQUIRE_EQ(pt.unresolved, 0); // private ...

      bool all_resolved{false};
      REQUIRE_NOTHROW(all_resolved = driver_pt.resolve_deps());
      REQUIRE(all_resolved);

      REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
      REQUIRE_EQ(built_targets.size(), 0);
    }

    SUBCASE("2 non-empty projects") {
      static std::string_view constexpr fake_root_file2{
          "/fake/dir/project2.root.cxx"};

      test_helpers::mock_project test_project2{
          &built_targets, nullptr, "dpttp2", "0.1.0", fake_root_file2};

      SUBCASE(
          "each with single phony target without cross-project dependencies") {
        auto *const pt_1{test_project1.add_mock_phony_target(fake_root_file1,
                                                             true, "pt_1", {})};
        REQUIRE_EQ(pt_1->resolved_name, "");

        auto *const pt_2{test_project2.add_mock_phony_target(fake_root_file2,
                                                             true, "pt_2", {})};
        REQUIRE_EQ(pt_2->resolved_name, "");

        REQUIRE_NOTHROW(driver_pt.process_project(&test_project1));
        REQUIRE_EQ(pt_1->resolved_name, "dpttp1::pt_1");

        REQUIRE_NOTHROW(driver_pt.process_project(&test_project2));
        REQUIRE_EQ(pt_2->resolved_name, "dpttp2::pt_2");

        REQUIRE_EQ(driver_pt.projects_by_name.size(), 2);
        REQUIRE_EQ(driver_pt.targets_by_project.size(), 2);
        REQUIRE_EQ(driver_pt.targets_by_project.begin()->second.size(), 1);
        REQUIRE_EQ(
            std::next(driver_pt.targets_by_project.begin())->second.size(), 1);
        REQUIRE_EQ(driver_pt.project_of_target.size(), 2);
        REQUIRE_EQ(driver_pt.targets_by_resolved_name.size(), 2);
        REQUIRE_EQ(driver_pt.intermediate_targets.size(), 0);
        // REQUIRE_EQ(pt.built_targets.size(), 0); // private ...
        REQUIRE_EQ(driver_pt.get_target_resolved_deps().size(), 2);
        // REQUIRE_EQ(pt.unresolved, 2); // private ...

        bool all_resolved{false};
        REQUIRE_NOTHROW(all_resolved = driver_pt.resolve_deps());
        REQUIRE(all_resolved);

        REQUIRE(built_targets.empty());
        REQUIRE_NOTHROW(driver_pt.build_target(pt_1, false));
        REQUIRE_EQ(built_targets.count(pt_1), 1);
        REQUIRE_EQ(built_targets.size(), 1);

        built_targets.clear();
        REQUIRE_NOTHROW(driver_pt.build_target(pt_2, false));
        REQUIRE_EQ(built_targets.count(pt_2), 1);
        REQUIRE_EQ(built_targets.size(), 1);
      }

      SUBCASE("phony target with one cross-project dependency") {
        auto *const pt_1{test_project1.add_mock_phony_target(
            fake_root_file1, false, "pt_1", {})};

        // will resolve dependency locally, relatively to parent its project
        auto *const pt_2{test_project1.add_mock_phony_target(
            fake_root_file1, true, "pt_2", {"pt_1"})};

        auto *const pt_3{test_project2.add_mock_phony_target(
            fake_root_file2, true, "pt_3", {"dpttp1::pt_1"})};

        REQUIRE_NOTHROW(driver_pt.process_project(&test_project1));
        REQUIRE_NOTHROW(driver_pt.process_project(&test_project2));

        bool all_resolved{false};
        REQUIRE_NOTHROW(all_resolved = driver_pt.resolve_deps());
        REQUIRE(all_resolved);

        REQUIRE(built_targets.empty());

        SUBCASE("build them separately") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          // without deps
          REQUIRE_NOTHROW(driver_pt.build_target(pt_1, false));
          REQUIRE_EQ(built_targets.count(pt_1), 1);
          REQUIRE_EQ(built_targets.size(), 1);

          // internal set will contain both ptrs, but this one is cleared in
          // order to have easier checks below
          built_targets.clear();

          // builds even its dependency
          REQUIRE_NOTHROW(driver_pt.build_target(pt_2, false));
          REQUIRE_EQ(*built_targets.begin(), pt_2);
          REQUIRE_EQ(built_targets.size(), 1);

          // ...
          built_targets.clear();

          // ...
          REQUIRE_NOTHROW(driver_pt.build_target(pt_3, false));
          REQUIRE_EQ(built_targets.count(pt_3), 1);
          REQUIRE_EQ(built_targets.size(), 1);
        }

        SUBCASE("build them together") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(pt_1), 1);
          REQUIRE_EQ(built_targets.count(pt_2), 1);
          REQUIRE_EQ(built_targets.count(pt_3), 1);
          REQUIRE_EQ(built_targets.size(), 3);
        }
      }
    }
  }

  SUBCASE("more nontrivial dependencies") {
    SUBCASE("file targets only") {
      SUBCASE("chain of 3 files, first 2 read-only") {
        auto *const f1{test_project1.add_mock_file_target(
            fake_root_file1, false, "/usr/include/foo.h", true, {})};

        auto *const f2{test_project1.add_mock_file_target(
            fake_root_file1, false, "src/fake.c", true,
            {"/usr/include/foo.h"})};

        auto *const f3{test_project1.add_mock_file_target(
            fake_root_file1, true, "bin/fake", false, {"src/fake.c"})};

        REQUIRE_NOTHROW(driver_pt.process_project(&test_project1));

        bool all_resolved{false};
        REQUIRE_NOTHROW(all_resolved = driver_pt.resolve_deps());
        REQUIRE(all_resolved);

        SUBCASE("all up-to date") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.touch(f1->get_resolved_path());

          fake_fs.touch(f2->get_resolved_path());

          fake_fs.touch(f3->get_resolved_path());

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.size(), 0);
        }

        SUBCASE("first newest") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.touch(f2->get_resolved_path());

          fake_fs.touch(f3->get_resolved_path());

          fake_fs.touch(f1->get_resolved_path());

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 1);
        }

        SUBCASE("second newest") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.touch(f3->get_resolved_path());

          fake_fs.touch(f1->get_resolved_path());

          fake_fs.touch(f2->get_resolved_path());

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 1);
        }

        SUBCASE("third doesn't exist") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.touch(f1->get_resolved_path());

          fake_fs.touch(f2->get_resolved_path());

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 1);
        }

        SUBCASE("same modification times") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.clock.freeze_time(true);

          fake_fs.touch(f1->get_resolved_path());

          fake_fs.touch(f2->get_resolved_path());

          fake_fs.touch(f3->get_resolved_path());

          fake_fs.clock.freeze_time(false);

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.size(), 0);
        }

        SUBCASE("third doesn't exist, otherwise same modification times") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.clock.freeze_time(true);

          fake_fs.touch(f1->get_resolved_path());

          fake_fs.touch(f2->get_resolved_path());

          fake_fs.clock.freeze_time(false);

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 1);
        }
      }

      SUBCASE("build-like tree ...") {
        auto *const f1s{test_project1.add_mock_file_target(
            fake_root_file1, false, "src/foo.c", true, {})};

        auto *const f1l{test_project1.add_mock_file_target(
            fake_root_file1, true, "bin/libfoo.a", false, {"src/foo.c"})};

        auto *const f2s{test_project1.add_mock_file_target(
            fake_root_file1, false, "src/bar.c", true, {})};

        auto *const f2l{test_project1.add_mock_file_target(
            fake_root_file1, true, "bin/libbar.a", false, {"src/bar.c"})};

        auto *const f3{test_project1.add_mock_file_target(
            fake_root_file1, true, "bin/fake", false,
            {"bin/libfoo.a", "bin/libbar.a"})};

        REQUIRE_NOTHROW(driver_pt.process_project(&test_project1));

        bool all_resolved{false};
        REQUIRE_NOTHROW(all_resolved = driver_pt.resolve_deps());
        REQUIRE(all_resolved);

        REQUIRE(built_targets.empty());

        SUBCASE("all up-to date") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.touch(f2s->get_resolved_path());

          fake_fs.touch(f1s->get_resolved_path());

          fake_fs.touch(f2l->get_resolved_path());

          fake_fs.touch(f1l->get_resolved_path());

          fake_fs.touch(f3->get_resolved_path());

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.size(), 0);
        }

        SUBCASE("first lib needs update") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.touch(f2s->get_resolved_path());

          fake_fs.touch(f2l->get_resolved_path());

          fake_fs.touch(f1l->get_resolved_path());

          fake_fs.touch(f1s->get_resolved_path());

          fake_fs.touch(f3->get_resolved_path());

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f1l), 1);
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 2);
        }

        SUBCASE("second lib needs update") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.touch(f2l->get_resolved_path());

          fake_fs.touch(f2s->get_resolved_path());

          fake_fs.touch(f1s->get_resolved_path());

          fake_fs.touch(f1l->get_resolved_path());

          fake_fs.touch(f3->get_resolved_path());

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f2l), 1);
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 2);
        }

        SUBCASE("first lib newest") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.touch(f1s->get_resolved_path());

          fake_fs.touch(f2s->get_resolved_path());

          fake_fs.touch(f2l->get_resolved_path());

          fake_fs.touch(f3->get_resolved_path());

          fake_fs.touch(f1l->get_resolved_path());

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 1);
        }

        SUBCASE("second lib newest") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.touch(f1s->get_resolved_path());

          fake_fs.touch(f2s->get_resolved_path());

          fake_fs.touch(f1l->get_resolved_path());

          fake_fs.touch(f3->get_resolved_path());

          fake_fs.touch(f2l->get_resolved_path());

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 1);
        }

        SUBCASE("third doesn't exist") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.touch(f1s->get_resolved_path());

          fake_fs.touch(f2s->get_resolved_path());

          fake_fs.touch(f2l->get_resolved_path());

          fake_fs.touch(f1l->get_resolved_path());

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 1);
        }

        SUBCASE("only RO files exist") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.touch(f1s->get_resolved_path());

          fake_fs.touch(f2s->get_resolved_path());

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f1l), 1);
          REQUIRE_EQ(built_targets.count(f2l), 1);
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 3);
        }

        SUBCASE("same modification times") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.clock.freeze_time(true);

          fake_fs.touch(f1s->get_resolved_path());

          fake_fs.touch(f2s->get_resolved_path());

          fake_fs.touch(f2l->get_resolved_path());

          fake_fs.touch(f3->get_resolved_path());

          fake_fs.touch(f1l->get_resolved_path());

          fake_fs.clock.freeze_time(false);

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.size(), 0);
        }

        SUBCASE("third doesn't exist, otherwise same modification times") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.clock.freeze_time(true);

          fake_fs.touch(f1s->get_resolved_path());

          fake_fs.touch(f2s->get_resolved_path());

          fake_fs.touch(f2l->get_resolved_path());

          fake_fs.touch(f1l->get_resolved_path());

          fake_fs.clock.freeze_time(false);

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 1);
        }

        SUBCASE(
            "third and second don't exist, otherwise same modification times") {
          SUBCASE("varying num. of workers") {
            SUBCASE("1") {
              // force 2 lines
              driver_pt.limit_to_n_workers(1);
            }
            SUBCASE("2") {
              // force 2 lines
              driver_pt.limit_to_n_workers(2);
            }
            SUBCASE("max") {
              // force 2 lines
              driver_pt.limit_to_n_workers(n_workers);
            }
          }

          fake_fs.clock.freeze_time(true);

          fake_fs.touch(f1s->get_resolved_path());

          fake_fs.touch(f2s->get_resolved_path());

          fake_fs.touch(f1l->get_resolved_path());

          fake_fs.clock.freeze_time(false);

          REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
          REQUIRE_EQ(built_targets.count(f2l), 1);
          REQUIRE_EQ(built_targets.count(f3), 1);
          REQUIRE_EQ(built_targets.size(), 2);
        }
      }

      // ...
      built_targets.clear();

      REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
      // all already up to date:
      REQUIRE_EQ(built_targets.size(), 0);
    }

    SUBCASE("relation between phony and file targets") {
      auto *const fro{test_project1.add_mock_file_target(
          fake_root_file1, false, "src/fake.c", true, {})};

      auto *const f1{test_project1.add_mock_file_target(
          fake_root_file1, true, "bin/fake1", false, {"src/fake.c"})};

      auto *const p1{test_project1.add_mock_phony_target(
          fake_root_file1, true, "phony_alias", {"bin/fake1"})};

      auto *const f2{test_project1.add_mock_file_target(
          fake_root_file1, true, "bin/fake2", false,
          {"src/fake.c", "fake_phony"})};

      auto *const p2{test_project1.add_mock_phony_target(fake_root_file1, true,
                                                         "fake_phony", {})};

      REQUIRE_NOTHROW(driver_pt.process_project(&test_project1));

      bool all_resolved{false};
      REQUIRE_NOTHROW(all_resolved = driver_pt.resolve_deps());
      REQUIRE(all_resolved);

      REQUIRE(built_targets.empty());

      SUBCASE("first, up to date") {
        SUBCASE("varying num. of workers") {
          SUBCASE("1") {
            // force 2 lines
            driver_pt.limit_to_n_workers(1);
          }
          SUBCASE("2") {
            // force 2 lines
            driver_pt.limit_to_n_workers(2);
          }
          SUBCASE("max") {
            // force 2 lines
            driver_pt.limit_to_n_workers(n_workers);
          }
        }

        fake_fs.touch(fro->get_resolved_path());

        fake_fs.touch(f1->get_resolved_path());

        REQUIRE_NOTHROW(driver_pt.build_target(p1, false));
        REQUIRE_EQ(built_targets.count(p1), 1);
        REQUIRE_EQ(built_targets.size(), 1);
      }

      SUBCASE("first, out of date") {
        SUBCASE("varying num. of workers") {
          SUBCASE("1") {
            // force 2 lines
            driver_pt.limit_to_n_workers(1);
          }
          SUBCASE("2") {
            // force 2 lines
            driver_pt.limit_to_n_workers(2);
          }
          SUBCASE("max") {
            // force 2 lines
            driver_pt.limit_to_n_workers(n_workers);
          }
        }

        fake_fs.touch(f1->get_resolved_path());

        fake_fs.touch(fro->get_resolved_path());

        REQUIRE_NOTHROW(driver_pt.build_target(p1, false));
        REQUIRE_EQ(built_targets.count(p1), 1);
        REQUIRE_EQ(built_targets.count(f1), 1);
        REQUIRE_EQ(built_targets.size(), 2);
      }

      SUBCASE("first, nonexistent") {
        SUBCASE("varying num. of workers") {
          SUBCASE("1") {
            // force 2 lines
            driver_pt.limit_to_n_workers(1);
          }
          SUBCASE("2") {
            // force 2 lines
            driver_pt.limit_to_n_workers(2);
          }
          SUBCASE("max") {
            // force 2 lines
            driver_pt.limit_to_n_workers(n_workers);
          }
        }

        fake_fs.touch(fro->get_resolved_path());

        REQUIRE_NOTHROW(driver_pt.build_target(p1, false));
        REQUIRE_EQ(built_targets.count(p1), 1);
        REQUIRE_EQ(built_targets.count(f1), 1);
        REQUIRE_EQ(built_targets.size(), 2);
      }

      SUBCASE("second, up to date") {
        SUBCASE("varying num. of workers") {
          SUBCASE("1") {
            // force 2 lines
            driver_pt.limit_to_n_workers(1);
          }
          SUBCASE("2") {
            // force 2 lines
            driver_pt.limit_to_n_workers(2);
          }
          SUBCASE("max") {
            // force 2 lines
            driver_pt.limit_to_n_workers(n_workers);
          }
        }

        fake_fs.touch(fro->get_resolved_path());

        fake_fs.touch(f2->get_resolved_path());

        REQUIRE_NOTHROW(driver_pt.build_target(f2, false));
        REQUIRE_EQ(built_targets.count(p2), 1);
        REQUIRE_EQ(built_targets.count(f2), 1);
        REQUIRE_EQ(built_targets.size(), 2);
      }

      SUBCASE("second, out of date") {
        SUBCASE("varying num. of workers") {
          SUBCASE("1") {
            // force 2 lines
            driver_pt.limit_to_n_workers(1);
          }
          SUBCASE("2") {
            // force 2 lines
            driver_pt.limit_to_n_workers(2);
          }
          SUBCASE("max") {
            // force 2 lines
            driver_pt.limit_to_n_workers(n_workers);
          }
        }

        fake_fs.touch(f2->get_resolved_path());

        fake_fs.touch(fro->get_resolved_path());

        REQUIRE_NOTHROW(driver_pt.build_target(f2, false));
        REQUIRE_EQ(built_targets.count(p2), 1);
        REQUIRE_EQ(built_targets.count(f2), 1);
        REQUIRE_EQ(built_targets.size(), 2);
      }

      SUBCASE("second, nonexistent") {
        SUBCASE("varying num. of workers") {
          SUBCASE("1") {
            // force 2 lines
            driver_pt.limit_to_n_workers(1);
          }
          SUBCASE("2") {
            // force 2 lines
            driver_pt.limit_to_n_workers(2);
          }
          SUBCASE("max") {
            // force 2 lines
            driver_pt.limit_to_n_workers(n_workers);
          }
        }

        fake_fs.touch(fro->get_resolved_path());

        REQUIRE_NOTHROW(driver_pt.build_target(f2, false));
        REQUIRE_EQ(built_targets.count(p2), 1);
        REQUIRE_EQ(built_targets.count(f2), 1);
        REQUIRE_EQ(built_targets.size(), 2);
      }
    }
  }

  // TODO verify: above cases for various nontrivial "graphs" of dependencies
  // should be "enough"
}

} // namespace
} // namespace build_cxx
