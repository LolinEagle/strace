#include "strace.h"

void	decode_access_mode(int mode)
{
	static const char *const	access_mode[5] = {
	[0] = "F_OK",
	[1] = "X_OK",
	[2] = "W_OK",
	[4] = "R_OK",
	};

	fprintf(stderr, "%s", access_mode[mode]);
}
