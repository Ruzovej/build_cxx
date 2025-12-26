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

#include "impl/cmd_wrapper.hxx"

#include <cerrno>
#include <csignal>
#include <cstring>

#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{

int check_errno(int const line, int const ret)
{
	if (errno != 0 || ret < 0)
	{
		throw std::runtime_error(
		  "syscall failed on line " + std::to_string(line) + " with code " + std::to_string(ret) + ", errno " +
		  std::to_string(errno) + " = \"" + std::strerror(errno) + "\"");
	}
	return ret;
};

#define CHECK(fn) check_errno(__LINE__, (fn))

class pipes
{
	static int constexpr invalid_fd = -1;

	int fds[2];

	static void close_fd(int &fd) noexcept
	{
		if (fd != invalid_fd)
		{
			// https://linux.die.net/man/2/close
			if (close(fd) == -1)
			{
				switch (errno)
				{
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
					std::cerr << "`close` failed - by https://linux.die.net/man/2/close unspecified `errno` " << errno
					          << "!\n";
					break;
				}
			}
			fd = invalid_fd;
		}
	}

  public:
	pipes()
	{
		CHECK(pipe(fds));
	}
	~pipes() noexcept
	{
		close_out();
		close_in();
	}

	pipes(const pipes &) = delete;
	pipes(pipes &&)      = delete;

	int get_out() const noexcept
	{
		return fds[0];
	}
	int get_in() const noexcept
	{
		return fds[1];
	}

	void close_out() noexcept
	{
		close_fd(fds[0]);
	}
	void close_in() noexcept
	{
		close_fd(fds[1]);
	}
};

}  // namespace

namespace exec_cmd
{

int bash(
  int const              timeout_ms,
  std::string_view const command,
  std::string_view const input,
  std::string *const     output,
  std::string *const     error,
  bool const             supress_logging)
{
	// Open pipes for input, output and error
	pipes input_pipe;

	pipes output_pipe;
	if (output)
	{
		output->clear();
	}

	std::optional<pipes> error_pipe;
	if (error != output)  // not redirected into single one
	{
		error_pipe.emplace();
	}
	if (error)
	{
		error->clear();
	}

	pid_t const pid = CHECK(fork());
	if (pid == 0)  // Child process
	{
		try
		{
			input_pipe.close_in();
			CHECK(dup2(input_pipe.get_out(), STDIN_FILENO));

			output_pipe.close_out();
			CHECK(dup2(output_pipe.get_in(), STDOUT_FILENO));

			if (error_pipe)
			{
				error_pipe->close_out();
				CHECK(dup2(error_pipe->get_in(), STDERR_FILENO));
			}
			else
			{
				CHECK(dup2(output_pipe.get_in(), STDERR_FILENO));
			}

			// Execute the command
			CHECK(execl("/bin/bash", "bash", "-c", command.data(), NULL));
		}
		catch (std::exception const &e)
		{
			std::cerr << "child process failed - caught exception: " << e.what() << '\n';
		}
		std::exit(EXIT_FAILURE);
	}
	else  // Parent process
	{
		if (!supress_logging)
		{
			std::cout << "parent process ... closing unused ends of the pipes\n";
		}
		input_pipe.close_out();
		output_pipe.close_in();
		if (error_pipe)
		{
			error_pipe->close_in();
		}

		if (!supress_logging)
		{
			std::cout << "parent process ... filling input pipe\n";
		}
		ssize_t written {0};
		while (written < static_cast<ssize_t>(input.length()))
		{
			auto const now_written =
			  write(input_pipe.get_in(), input.data() + written, input.length() - static_cast<size_t>(written));
			written += now_written;
		}
		input_pipe.close_in();

		auto const read_pipes = [&]() {
			auto const read_pipe = [supress_logging](int const fd, std::string &buffer) {
				int avail = 0;
				CHECK(ioctl(fd, FIONREAD, &avail));

				ssize_t nbytes {0};
				if (avail > 0)  // TODO read in a loop (in case `nbytes` < `avail`)
				{
					buffer.resize(static_cast<size_t>(avail));

#if defined(__clang__)  // TODO why the `if constexpr ...` doesn't work?!
					nbytes = read(fd, const_cast<char *>(buffer.data()), buffer.size());
#else
					nbytes = read(fd, buffer.data(), buffer.size());
#endif
				}
				buffer.shrink_to_fit();
				if (nbytes < avail)
				{
					throw std::runtime_error("failed to read all available bytes from given pipe!");
				}
				if (!supress_logging)
				{
					std::cout << "parent process ... reads " << nbytes << " [B] from pipe (where " << avail
					          << " [B] were available)\n";
				}
			};

			if (!supress_logging)
			{
				std::cout << "parent process ... reading child's outputs\n";
			}

			if (output != error)
			{
				if (output)
				{
					read_pipe(output_pipe.get_out(), *output);
				}
				if (error)
				{
					read_pipe(error_pipe->get_out(), *error);
				}
			}
			else if (output)
			{
				read_pipe(output_pipe.get_out(), *output);
			}
		};

		if (!supress_logging)
		{
			std::cout << "parent process ... waiting for the child process to finish\n";
		}

		struct pollfd p_fd;
		p_fd.fd            = CHECK(static_cast<int>(syscall(
          static_cast<long>(SYS_pidfd_open), pid, 0)));  // https://man7.org/linux/man-pages/man2/pidfd_open.2.html
		p_fd.events        = POLLIN;
		int const poll_res = CHECK(poll(&p_fd, 1, timeout_ms));  // https://stackoverflow.com/a/65003348/10712915
		if (poll_res == 0)                                       // timed out
		{
			read_pipes();
			CHECK(kill(pid, SIGTERM));
			throw std::runtime_error("command timed out, child process was terminated");
		}
		int status;
		CHECK(waitpid(pid, &status, WNOHANG));  // https://linux.die.net/man/2/waitpid

		read_pipes();

		// Return the exit status of the child process
		return WEXITSTATUS(status);
	}
}

}  // namespace exec_cmd
