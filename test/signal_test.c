#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void	handler(int sig)
{
	const char	msg[] = "Caught SIGUSR1\n";

	(void)sig;
	write(1, msg, sizeof(msg) - 1);
}

void	segv(void)
{
	volatile int	*ptr;

	ptr = NULL;
	*ptr = 42;// Triggers SIGSEGV
}

int	main(int argc, char **argv)
{
	volatile int	a;
	volatile int	b;
	int				c;

	if (argc < 2)
		return (1);

	if (strcmp(argv[1], "segv") == 0)
		segv();
	else if (strcmp(argv[1], "fpe") == 0)
	{
		a = 1;
		b = 0;
		c = a / b; // Triggers SIGFPE
		(void)c;
	}
	else if (strcmp(argv[1], "usr1") == 0)
	{
		signal(SIGUSR1, handler);
		raise(SIGUSR1);
	}
	else if (strcmp(argv[1], "term") == 0)
		kill(getpid(), SIGTERM);
	return (0);
}
