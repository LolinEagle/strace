#include "ft_strace.h"

static volatile sig_atomic_t	g_interrupted = 0;

int	validate_command(const char *cmd, int found)
{
	struct stat	sb;
	const char	*path_env;
	char		*path_copy;
	char		full_path[PATH_MAX];
	char		*dir;

	// Case 1: Path contains '/' (e.g., "/bin/fake_cmd", "./my_prog")
	if (strchr(cmd, '/'))
	{
		if (stat(cmd, &sb) == -1)
		{
			fprintf(stderr, "strace: Cannot stat '%s': %s\n", cmd,
				strerror(errno));
			return (-1);
		}
		return (0);
	}

	// Case 2: No '/' provided (e.g., "fake_cmd") -> Search inside PATH
	path_env = getenv("PATH");
	if (!path_env || path_env[0] == '\0')
		path_env = "."; // Default to current directory if PATH is empty

	path_copy = strdup(path_env);
	if (!path_copy)
	{
		perror("strdup");
		return (-1);
	}

	dir = strtok(path_copy, ":");

	while (dir != NULL)
	{
		snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);
		if (stat(full_path, &sb) == 0)
		{
			found = 1;
			break ;
		}
		dir = strtok(NULL, ":");
	}

	free(path_copy);

	if (!found)
	{
		// When not found anywhere in PATH, stat() fails with ENOENT
		fprintf(stderr, "strace: Cannot stat '%s': %s\n", cmd,
			strerror(ENOENT));
		return (-1);
	}
	return (0);
}

static void	sigint_handler(int sig)
{
	(void)sig;
	g_interrupted = 1;
}

int	main(int argc, char **argv, char **envp)
{
	struct sigaction	sa;
	int					sync_pipe[2];
	pid_t				pid;
	char				sync_buf;

	if (argc < 2)
	{
		fprintf(stderr, "ft_strace: must have PROG [ARGS]\n");
		return (1);
	}
	if (validate_command(argv[1], 0) != 0)
		return (1);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa, NULL);// Signal Interrupt

	if (pipe(sync_pipe) == -1)
	{
		perror("ft_strace: pipe");
		return (1);
	}

	pid = fork();
	if (pid < 0)
	{
		perror("ft_strace: fork");
		return (1);
	}

	if (pid == 0)
	{
		// Child process
		close(sync_pipe[1]);

		// Wait for tracer to perform PTRACE_SEIZE
		if (read(sync_pipe[0], &sync_buf, 1) != 1)
		{
			close(sync_pipe[0]);
			exit(1);
		}
		close(sync_pipe[0]);

		// Stop self to hand off control cleanly to tracer
		kill(getpid(), SIGSTOP);

		execvpe(argv[1], &argv[1], envp);
		perror("strace: exec");
		exit(1);
	}

	// Parent / Tracer process
	close(sync_pipe[0]);

	// Attach to child using PTRACE_SEIZE
	if (ptrace(PTRACE_SEIZE, pid, 0, (void *)PTRACE_O_TRACESYSGOOD) == -1)
	{
		perror("ft_strace: ptrace(PTRACE_SEIZE)");
		close(sync_pipe[1]);
		kill(pid, SIGKILL);
		return (1);
	}

	// Signal child to proceed to SIGSTOP
	if (write(sync_pipe[1], "GO", 1) != 1)
	{
		perror("ft_strace: write");
		close(sync_pipe[1]);
		kill(pid, SIGKILL);
		return (1);
	}
	close(sync_pipe[1]);
	return (run_tracer(pid, argc, argv, envp));
}
