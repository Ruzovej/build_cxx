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

#include "build_cxx/common/target_status.hxx"

namespace build_cxx::test_helpers {

struct fake_clock {
  using time_ns_t = common::target_status::file_modification_time_t;

  [[nodiscard]] time_ns_t now_ns(bool const current = false) {
    return (current || time_frozen) ? time_ns : (++time_ns);
  }

  void freeze_time(bool const freeze) { time_frozen = freeze; }

private:
  time_ns_t time_ns{};
  bool time_frozen{false};
};

} // namespace build_cxx::test_helpers
