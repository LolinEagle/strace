#include <unistd.h>
#include <fcntl.h>

int	main(void)
{
	int	fd;

	write(1, "Hello 64\n", 9);
	fd = open("/dev/null", O_RDONLY);
	if (fd >= 0)
		close(fd);
	return (42);
}
