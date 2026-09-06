#include <signal.h>
#include <unistd.h>

void	handler(int s)
{
	(void)s;
}

int	main(void)
{
	signal(SIGALRM, handler);
	alarm(1);
	pause();
	return (0);
}
