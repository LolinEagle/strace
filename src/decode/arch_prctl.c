#include "strace.h"

void	decode_arch_prctl_op(int op)
{
	if (op == ARCH_GET_CPUID)
		fprintf(stderr, "ARCH_GET_CPUID");
	else if (op == ARCH_SET_FS)
		fprintf(stderr, "ARCH_SET_FS");
	else if (op == ARCH_GET_FS)
		fprintf(stderr, "ARCH_GET_FS");
	else if (op == ARCH_SET_GS)
		fprintf(stderr, "ARCH_SET_GS");
	else if (op == ARCH_GET_GS)
		fprintf(stderr, "ARCH_GET_GS");
}
