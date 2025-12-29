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

#include "impl/exec_path_args.hxx"

// TODO cleanup includes later:

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstring>

#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>

#include "impl/syscall_helper.hxx"

// TODO delete later ...:
/*
namespace {

int check_errno(int const line, int const ret) {
  if (errno != 0 || ret < 0) {
    throw std::runtime_error("syscall failed on line " + std::to_string(line) +
                             " with code " + std::to_string(ret) + ", errno " +
                             std::to_string(errno) + " = \"" +
                             std::strerror(errno) + "\"");
  }
  return ret;
};

#define CHECK(fn) check_errno(__LINE__, (fn))

class pipes {
  static int constexpr invalid_fd = -1;

  int fds[2];

  static void close_fd(int &fd) noexcept {
    if (fd != invalid_fd) {
      // https://linux.die.net/man/2/close
      if (close(fd) == -1) {
        switch (errno) {
        case EBADF:
          std::cerr << "`close` failed - invalid file descriptor!\n";
          break;
        case EINTR:
          std::cerr << "`close` failed - was interrupted by a signal!\n";
          break;
        case EIO:
          std::cerr << "`close` failed - I/O error occurred!\n";
          break;
        default:
          std::cerr << "`close` failed - by https://linux.die.net/man/2/close "
                       "unspecified `errno` "
                    << errno << "!\n";
          break;
        }
      }
      fd = invalid_fd;
    }
  }

public:
  pipes() { CHECK(pipe(fds)); }
  ~pipes() noexcept {
    close_out();
    close_in();
  }

  pipes(const pipes &) = delete;
  pipes(pipes &&) = delete;

  int get_out() const noexcept { return fds[0]; }
  int get_in() const noexcept { return fds[1]; }

  void close_out() noexcept { close_fd(fds[0]); }
  void close_in() noexcept { close_fd(fds[1]); }
};

} // namespace

namespace exec_cmd {

int bash(int const timeout_ms, std::string_view const command,
         std::string_view const input, std::string *const output,
         std::string *const error, bool const supress_logging) {
  // Open pipes for input, output and error
  pipes input_pipe;

  pipes output_pipe;
  if (output) {
    output->clear();
  }

  std::optional<pipes> error_pipe;
  if (error != output) // not redirected into single one
  {
    error_pipe.emplace();
  }
  if (error) {
    error->clear();
  }

  // 1

  pid_t const pid = CHECK(fork());
  if (pid == 0) // Child process
  {
    try {
      input_pipe.close_in();
      CHECK(dup2(input_pipe.get_out(), STDIN_FILENO));

      output_pipe.close_out();
      CHECK(dup2(output_pipe.get_in(), STDOUT_FILENO));

      if (error_pipe) {
        error_pipe->close_out();
        CHECK(dup2(error_pipe->get_in(), STDERR_FILENO));
      } else {
        CHECK(dup2(output_pipe.get_in(), STDERR_FILENO));
      }

      // Execute the command
      CHECK(execl("/bin/bash", "bash", "-c", command.data(), NULL));
    } catch (std::exception const &e) {
      std::cerr << "child process failed - caught exception: " << e.what()
                << '\n';
    }
    std::exit(EXIT_FAILURE);
  } else // Parent process
  {
    if (!supress_logging) {
      std::cout << "parent process ... closing unused ends of the pipes\n";
    }
    input_pipe.close_out();
    output_pipe.close_in();
    if (error_pipe) {
      error_pipe->close_in();
    }

    // 2

    if (!supress_logging) {
      std::cout << "parent process ... filling input pipe\n";
    }
    ssize_t written{0};
    while (written < static_cast<ssize_t>(input.length())) {
      auto const now_written =
          write(input_pipe.get_in(), input.data() + written,
                input.length() - static_cast<size_t>(written));
      written += now_written;
    }
    input_pipe.close_in();

    // 3

    auto const read_pipes = [&]() {
      auto const read_pipe = [supress_logging](int const fd,
                                               std::string &buffer) {
        int avail = 0;
        CHECK(ioctl(fd, FIONREAD, &avail));

        ssize_t nbytes{0};
        if (avail > 0) // TODO read in a loop (in case `nbytes` < `avail`)
        {
          buffer.resize(static_cast<size_t>(avail));

#if defined(__clang__) // TODO why the `if constexpr ...` doesn't work?!
          nbytes = read(fd, const_cast<char *>(buffer.data()), buffer.size());
#else
          nbytes = read(fd, buffer.data(), buffer.size());
#endif
        }
        buffer.shrink_to_fit();
        if (nbytes < avail) {
          throw std::runtime_error(
              "failed to read all available bytes from given pipe!");
        }
        if (!supress_logging) {
          std::cout << "parent process ... reads " << nbytes
                    << " [B] from pipe (where " << avail
                    << " [B] were available)\n";
        }
      };

      if (!supress_logging) {
        std::cout << "parent process ... reading child's outputs\n";
      }

      if (output != error) {
        if (output) {
          read_pipe(output_pipe.get_out(), *output);
        }
        if (error) {
          read_pipe(error_pipe->get_out(), *error);
        }
      } else if (output) {
        read_pipe(output_pipe.get_out(), *output);
      }
    };

    if (!supress_logging) {
      std::cout
          << "parent process ... waiting for the child process to finish\n";
    }

    struct pollfd p_fd;
    p_fd.fd = CHECK(static_cast<int>(
        syscall(static_cast<long>(SYS_pidfd_open), pid,
                0))); // https://man7.org/linux/man-pages/man2/pidfd_open.2.html
    p_fd.events = POLLIN;
    int const poll_res = CHECK(poll(
        &p_fd, 1, timeout_ms)); // https://stackoverflow.com/a/65003348/10712915
    if (poll_res == 0)          // timed out
    {
      read_pipes();
      CHECK(kill(pid, SIGTERM));
      throw std::runtime_error(
          "command timed out, child process was terminated");
    }
    int status;
    CHECK(
        waitpid(pid, &status, WNOHANG)); // https://linux.die.net/man/2/waitpid

    read_pipes();

    // Return the exit status of the child process
    return WEXITSTATUS(status);
  }
}

} // namespace exec_cmd*/

namespace build_cxx::os_wrapper {

exec_path_args::exec_path_args(exec_path_args &&rhs) noexcept
    : path{std::move(rhs.path)}, args{std::move(rhs.args)},
      time_spawned_ms{rhs.time_spawned_ms},
      time_finished_ms{rhs.time_finished_ms}, handle{std::exchange(
                                                  rhs.handle,
                                                  invalid_process_handle)},
      stdin_pipe{std::move(rhs.stdin_pipe)},
      stdout_pipe{std::move(rhs.stdout_pipe)}, stderr_pipe{std::move(
                                                   rhs.stderr_pipe)},
      current_state{std::exchange(rhs.current_state, state::uninitialzied)},
      return_code{rhs.return_code}, stdout_buffer{std::move(rhs.stdout_buffer)},
      stdout_consumed_bytes{rhs.stdout_consumed_bytes}, stderr_buffer{std::move(
                                                            rhs.stderr_buffer)},
      stderr_consumed_bytes{rhs.stderr_consumed_bytes} {}

exec_path_args::~exec_path_args() noexcept {
  do_kill(); // if this throws ... just let the OS "abort us".
}

exec_path_args::states
exec_path_args::update_and_get_state(int const timeout_until_it_finishes_ms) {
  auto const previous_state{current_state};

  switch (current_state) {
  case state::ready: {
    stdin_pipe.init();
    stdout_pipe.init();
    stderr_pipe.init();

    static_assert(std::is_same_v<pid_t, process_handle_t>);
    pid_t const pid{BUILD_CXX_SYSCALL_HELPER(fork())};
    if (pid == 0) // Child process
    {
      try {
        stdin_pipe.close_in();
        BUILD_CXX_SYSCALL_HELPER(dup2(stdin_pipe.get_out(), STDIN_FILENO));

        stdout_pipe.close_out();
        BUILD_CXX_SYSCALL_HELPER(dup2(stdout_pipe.get_in(), STDOUT_FILENO));

        stderr_pipe.close_out();
        BUILD_CXX_SYSCALL_HELPER(dup2(stderr_pipe.get_in(), STDERR_FILENO));

        auto const num_args{args.size()};
        // auto const args_cstr{std::make_unique<char *[]>(num_args + 2)};
        auto const args_cstr{std::make_unique<char *[]>(num_args + 1)};
        // https://man7.org/linux/man-pages/man3/exec.3.html -> "The first
        // argument, by convention, should point to the filename associated with
        // the file being executed"
        // TODO:
        // 1. follow this convention, right off the bat it didin't work
        // 2. remove/"prevent" this `const_cast`
        // args_cstr[0] = const_cast<char *>(path.c_str());
        for (std::size_t i{0}; i < num_args; ++i) {
          // TODO ... remove/"prevent" this `const_cast`
          // args_cstr[i + 1] = const_cast<char *>(args[i].c_str());
          args_cstr[i] = const_cast<char *>(args[i].c_str());
        }
        // args_cstr[num_args + 1] = nullptr;
        args_cstr[num_args] = nullptr;

        // Execute the command
        BUILD_CXX_SYSCALL_HELPER(execv(path.c_str(), args_cstr.get()));
      } catch (std::exception const &e) {
        std::cerr << "child process failed - caught exception: " << e.what()
                  << '\n';
      }
      std::exit(EXIT_FAILURE);
    }
    // else ... parent process
    current_state = state::running;
    handle = pid;
    time_spawned_ms = -1; // TODO get current time in ms

    stdin_pipe.close_out();
    stdout_pipe.close_in();
    stderr_pipe.close_in();

    if (timeout_until_it_finishes_ms != 0) {
      // IMHO safer than "fallthrough":
      return {previous_state,
              update_and_get_state(timeout_until_it_finishes_ms).current};
    }

    break;
  }
  case state::running: {
    // https://man7.org/linux/man-pages/man2/pidfd_open.2.html
    auto const pid_fd{BUILD_CXX_SYSCALL_HELPER(static_cast<int>(
        syscall(static_cast<long>(SYS_pidfd_open), handle, 0)))};

    pollfd p_fd{pid_fd, POLLIN};

    // https://man7.org/linux/man-pages/man2/poll.2.html
    // https://stackoverflow.com/a/65003348/10712915
    auto const poll_res{
        BUILD_CXX_SYSCALL_HELPER(poll(&p_fd, 1, timeout_until_it_finishes_ms))};

    if (poll_res == 1) {
      query_status(false);
    }

    if ((timeout_until_it_finishes_ms < 0) &&
        (current_state != state::finished)) {
      throw std::runtime_error{
          "failed to wait for child process to finish without any timeout!"};
    }

    break;
  }
  case state::finished: {
    // whatever ... nothing seems necessary here
    break;
  }
  case state::uninitialzied: {
    throw std::runtime_error{
        "cannot update state - process wasn't initialized!"};
  }
  default: {
    throw std::runtime_error{"unknown state!"};
  }
  }

  return {previous_state, current_state};
}

void exec_path_args::send_to_stdin(std::string_view const data) {
  if (current_state != state::running) {
    throw std::runtime_error{
        "cannot write to inferior stdin - process isn't running!"};
  }

  ssize_t written{0};
  auto const data_size{static_cast<ssize_t>(
      data.size())}; // over-simplified version ... since
                     // `std::ssize` for `std::string_view` is C++20

  while (written < data_size) {
    auto const now_written{BUILD_CXX_SYSCALL_HELPER(write(
        stdin_pipe.get_in(), data.data() + written, data_size - written))};
    written += now_written;
  }
}

void exec_path_args::close_stdin() {
  if (current_state != state::running || stdin_pipe.get_in() == invalid_fd) {
    throw std::runtime_error{
        "cannot close inferior stdin - process isn't running or invalid fd!"};
  }
  stdin_pipe.close_in();
}

void exec_path_args::do_kill() {
  if (current_state == state::running) {
    BUILD_CXX_SYSCALL_HELPER(kill(handle, SIGKILL));
    query_status(true);
  }
}

long long exec_path_args::time_running_ms() const {
  if (current_state == state::running) {
    return 0; // TODO `"current time in ms" - time_spawned_ms`

  } else if (current_state == state::finished) {
    return time_finished_ms - time_spawned_ms;
  } else {
    throw std::runtime_error{
        "cannot get running time - process isn't running or finished!"};
  }
}

int exec_path_args::get_return_code() const {
  if (current_state != state::finished) {
    throw std::runtime_error{
        "cannot get return code - process isn't finished!"};
  }
  return return_code;
}

void exec_path_args::query_status(bool const wait_for_finishing) {
  if (current_state == state::running) {
    siginfo_t status{};
    int const options{WEXITED | (wait_for_finishing ? 0 : WNOHANG)};
    //  https://linux.die.net/man/2/waitpid
    BUILD_CXX_SYSCALL_HELPER(waitid(P_PID, handle, &status, options));

    if ((status.si_pid != 0) &&
        ((status.si_code == CLD_EXITED) || (status.si_code == CLD_KILLED) ||
         (status.si_code == CLD_DUMPED))) {
      if (status.si_pid != handle) {
        throw std::runtime_error{"waitid returned unexpected pid different "
                                 "from the managed one !"};
      }
      time_finished_ms = -1; // TODO get current time in ms
      current_state = state::finished;
      return_code =
          status.si_status; // or signal ... don't make a difference here
    }
  } else if (current_state == state::finished) {
    return;
  } else {
    throw std::runtime_error{
        "cannot wait for pid - process isn't running or finished!"};
  }
}

void exec_path_args::update_buffer(bool const for_stdout) {
  static auto constexpr read_pipe = [](int const fd, std::string &buffer) {
    auto const buf_prev_size{static_cast<ssize_t>(buffer.size())};

    int avail{0};
    BUILD_CXX_SYSCALL_HELPER(ioctl(fd, FIONREAD, &avail));

    ssize_t nbytes{0};
    if (0 < avail) // TODO read in a loop (in case `nbytes` < `avail`)?!
    {
      buffer.resize(static_cast<size_t>(buf_prev_size + avail));

#if defined(__clang__)
      // TODO get rid of the ugly `const_cast`
      nbytes = BUILD_CXX_SYSCALL_HELPER(
          read(fd, const_cast<char *>(buffer.data()) + buf_prev_size, avail));
#else
      nbytes = BUILD_CXX_SYSCALL_HELPER(
          read(fd, buffer.data() + buf_prev_size, avail));
#endif
    }
    if (nbytes < avail) {
      throw std::runtime_error(
          "failed to read all available bytes from given pipe!");
    }
    // std::cout << "parent process ... reads " << nbytes
    //           << " [B] from pipe (where " << avail
    //           << " [B] were available)\n";
  };

  read_pipe(for_stdout ? stdout_pipe.get_out() : stderr_pipe.get_out(),
            for_stdout ? stdout_buffer : stderr_buffer);
}

} // namespace build_cxx::os_wrapper
