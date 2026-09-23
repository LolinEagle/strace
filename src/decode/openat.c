#include "strace.h"

void	decode_openat_dirfd(int dirfd)
{
	if (dirfd == AT_FDCWD)
		fprintf(stderr, "AT_FDCWD");
	else
		fprintf(stderr, "%i", dirfd);
}

void	decode_openat_flags(int flags)
{
	bool	first;
	int		access_mode;

	// Handle access mode first
	access_mode = flags & O_ACCMODE;
	if (access_mode == O_RDONLY)
		fprintf(stderr, "O_RDONLY");
	else if (access_mode == O_WRONLY)
		fprintf(stderr, "O_WRONLY");
	else if (access_mode == O_RDWR)
		fprintf(stderr, "O_RDWR");
	first = false;

	if (flags & O_CREAT)
		print_flag("O_CREAT", &first);
	if (flags & O_EXCL)
		print_flag("O_EXCL", &first);
	if (flags & O_NOCTTY)
		print_flag("O_NOCTTY", &first);
	if (flags & O_TRUNC)
		print_flag("O_TRUNC", &first);
	if (flags & O_APPEND)
		print_flag("O_APPEND", &first);
	if (flags & O_NONBLOCK)
		print_flag("O_NONBLOCK", &first);
	if (flags & O_DSYNC)
		print_flag("O_DSYNC", &first);
	if (flags & FASYNC)
		print_flag("FASYNC", &first);
	if (flags & O_DIRECT)
		print_flag("O_DIRECT", &first);
	if (flags & O_LARGEFILE)
		print_flag("O_LARGEFILE", &first);
	if (flags & O_DIRECTORY)
		print_flag("O_DIRECTORY", &first);
	if (flags & O_NOFOLLOW)
		print_flag("O_NOFOLLOW", &first);
	if (flags & O_NOATIME)
		print_flag("O_NOATIME", &first);
	if (flags & O_CLOEXEC)
		print_flag("O_CLOEXEC", &first);
	if (flags & O_SYNC)
		print_flag("O_SYNC", &first);
	if (flags & O_PATH)
		print_flag("O_PATH", &first);
	if (flags & O_TMPFILE)
		print_flag("O_TMPFILE", &first);
}
