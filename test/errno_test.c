#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int	main(void)
{
	// Should return -1 (ENOENT = 2)
	open("/non/existent/file/path", O_RDONLY);

	// Should return -1 (EBADF = 9)
	close(999);

	// Should return -1 (EACCES = 13 or ENOENT)
	access("/root/secret_file_xyz", R_OK);
	return (0);
}
