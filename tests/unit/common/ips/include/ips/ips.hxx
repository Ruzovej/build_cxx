/*
  Copyright 2025 Lukáš Růžička

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

// due to https://claude.ai/share/458d69e0-9519-4e20-91eb-8c99c7dd9d0a

#pragma once

#include <string>

struct ips {
  // Both processes must use the same name to connect to each other
  explicit ips(std::string const &aName, bool const create);
  ~ips() noexcept;

  // Wait for notification. Returns true if notified, false on timeout.
  bool wait(int const timeout_ms);

  void notify();

  bool notify_and_wait(int const timeout_ms) {
    notify();
    return wait(timeout_ms);
  }

  bool wait_and_notify(int const timeout_ms) {
    bool const res{wait(timeout_ms)};
    notify();
    return res;
  }

private:
  ips(ips const &) = delete;
  ips &operator=(ips const &) = delete;

  void *sem;
  std::string name;
  bool owns; // responsible for cleanup
};
