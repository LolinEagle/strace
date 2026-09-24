#include "strace.h"

void	decode_getrandom_flags(int flags)
{
	if (flags == GRND_RANDOM)
		fprintf(stderr, "GRND_RANDOM");
	else if (flags == GRND_NONBLOCK)
		fprintf(stderr, "GRND_NONBLOCK");
	else
		fprintf(stderr, "%i", flags);
}
