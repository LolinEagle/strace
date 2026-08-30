#include <unistd.h>
#include <stdlib.h>

int	main(void)
{
	const char	msg[] = "Hello from test!\n";

	write(1, msg, sizeof(msg) - 1);
	getpid();
	return (42);
}
