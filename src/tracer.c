#include "ft_strace.h"

static int	get_registers(t_tracer *t)
{
	struct iovec	iov;

	iov.iov_base = &t->regs64;
	iov.iov_len = sizeof(t->regs64);

	if (ptrace(PTRACE_GETREGSET, t->child_pid, (void *)(uintptr_t)NT_PRSTATUS,
		&iov) == -1)
		return (-1);

	if (iov.iov_len == sizeof(t_user_regs_struct32))
	{
		t->arch = ARCH_32;
		memcpy(&t->regs32, &t->regs64, sizeof(t_user_regs_struct32));
	}
	else
		t->arch = ARCH_64;
	return (0);
}

int	run_tracer(pid_t child_pid)
{
	int			status;
	t_tracer	t;
	int			sig;
	siginfo_t	si;

	memset(&t, 0, sizeof(t));
	t.child_pid = child_pid;
	t.in_syscall = false;

	// Wait for child process to stop on initial SIGSTOP
	if (waitpid(child_pid, &status, WUNTRACED) == -1)
	{
		perror("ft_strace: waitpid");
		return (1);
	}

	// Configure options: set TRACESYSGOOD to distinguish syscall stops
	if (ptrace(PTRACE_SETOPTIONS, child_pid, 0, (void *)(PTRACE_O_TRACESYSGOOD))
			== -1)
	{
		perror("ft_strace: ptrace(PTRACE_SETOPTIONS)");
		return (1);
	}

	// Step until the first system call
	if (ptrace(PTRACE_SYSCALL, child_pid, 0, 0) == -1)
	{
		perror("ft_strace: ptrace(PTRACE_SYSCALL)");
		return (1);
	}

	while (1)
	{
		if (waitpid(child_pid, &status, 0) == -1)
		{
			// Interrupted system call
			if (errno == EINTR)
				continue ;
			break ;
		}

		if (WIFEXITED(status))
		{
			if (t.in_syscall)
				fprintf(stderr, " = ?\n");
			fprintf(stderr, "+++ exited with %d +++\n", WEXITSTATUS(status));
			return (WEXITSTATUS(status));
		}

		if (WIFSIGNALED(status))
		{
			if (t.in_syscall)
				fprintf(stderr, " = ?\n");
			fprintf(stderr, "+++ killed by %s (core dumped) +++\n",
				get_signal_name(WTERMSIG(status)));
			return (128 + WTERMSIG(status));
		}

		if (WIFSTOPPED(status))
		{
			sig = WSTOPSIG(status);
			if (sig == (SIGTRAP | 0x80))
			{
				if (get_registers(&t) == -1)
					break ;

				if (!t.in_syscall)
				{
					print_syscall_entry(&t);
					t.in_syscall = true;
				}
				else
				{
					print_syscall_exit(&t);
					t.in_syscall = false;
				}
				sig = 0;
			}
			else if (sig != SIGTRAP && sig != SIGSTOP)
			{
				memset(&si, 0, sizeof(si));
				if (ptrace(PTRACE_GETSIGINFO, child_pid, 0, &si) != -1)
					print_signal(&si);
			}
			else if (sig == SIGSTOP)
				sig = 0;

			if (ptrace(PTRACE_SYSCALL, child_pid, 0, (void *)(uintptr_t)sig)
					== -1)
				break ;
		}
	}
	return (0);
}
