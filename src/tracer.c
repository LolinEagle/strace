#include "strace.h"

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

void	tracer_signal(t_tracer *t, int child_pid)
{
	siginfo_t	si;

	memset(&si, 0, sizeof(si));
	if (ptrace(PTRACE_GETSIGINFO, child_pid, 0, &si) != -1)
		print_signal(t, &si);
}

int	run_tracer(pid_t child_pid, int argc, char **argv, char **envp)
{
	int			status;
	t_tracer	t;
	int			sig;
	sigset_t	empty;
	sigset_t	blocked;

	memset(&t, 0, sizeof(t));
	t.argc = argc;
	t.argv = argv;
	t.envp = envp;
	t.child_pid = child_pid;
	t.in_syscall = false;

	// Initialize sigsets as per the requirements
	sigemptyset(&empty);
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGHUP);
	sigaddset(&blocked, SIGQUIT);
	sigaddset(&blocked, SIGPIPE);
	sigaddset(&blocked, SIGTERM);

	// Wait for child process to stop on initial SIGSTOP
	sigprocmask(SIG_SETMASK, &empty, NULL);
	if (waitpid(child_pid, &status, WUNTRACED) == -1)
	{
		perror("strace: waitpid");
		return (1);
	}
	sigprocmask(SIG_BLOCK, &blocked, NULL);

	// Configure options: set TRACESYSGOOD to distinguish syscall stops
	if (ptrace(PTRACE_SETOPTIONS, child_pid, 0, (void *)(PTRACE_O_TRACESYSGOOD))
			== -1)
	{
		perror("strace: ptrace(PTRACE_SETOPTIONS)");
		return (1);
	}

	// Step until the first system call
	if (ptrace(PTRACE_SYSCALL, child_pid, 0, 0) == -1)
	{
		perror("strace: ptrace(PTRACE_SYSCALL)");
		return (1);
	}

	while (1)
	{
		sigprocmask(SIG_SETMASK, &empty, NULL);
		sig = waitpid(child_pid, &status, 0);
		sigprocmask(SIG_BLOCK, &blocked, NULL);
		if (sig == -1)
		{
			// Interrupted system call
			if (errno == EINTR)
				continue ;
			break ;
		}

		if (WIFEXITED(status))
		{
			if (t.in_syscall)
				fprintf(stderr, RESET ") = ?\n");
			fprintf(stderr, "+++ exited with %d +++\n", WEXITSTATUS(status));
			return (WEXITSTATUS(status));
		}

		if (WIFSIGNALED(status))
		{
			if (t.in_syscall)
				fprintf(stderr, " = ?\n");
			sig = WTERMSIG(status);
			if (WCOREDUMP(status))
				fprintf(stderr, "+++ killed by %s (core dumped) +++\n",
					get_signal_name(sig));
			else
				fprintf(stderr, "+++ killed by %s +++\n", get_signal_name(sig));
			return (128 + sig);
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
					print_syscall_entry(&t, true);
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
				tracer_signal(&t, child_pid);
			else if (sig == SIGSTOP)
				sig = 0;

			if (ptrace(PTRACE_SYSCALL, child_pid, 0, (void *)(uintptr_t)sig)
					== -1)
				break ;
		}
	}
	return (0);
}
