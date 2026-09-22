#include "strace.h"

void	print_flag(const char *str, bool *first)
{
	if (!*first)
		fprintf(stderr, "|");
	fprintf(stderr, "%s", str);
	*first = false;
}

void	decode_mmap_prot(int prot)
{
	bool	first;

	if (prot == PROT_NONE)
	{
		fprintf(stderr, "PROT_NONE");
		return ;
	}

	first = true;
	if (prot & PROT_READ)
	{
		fprintf(stderr, "PROT_READ");
		first = false;
	}
	if (prot & PROT_WRITE)
		print_flag("PROT_WRITE", &first);
	if (prot & PROT_EXEC)
		print_flag("PROT_EXEC", &first);
}

void	decode_mmap_flags(int flags)
{
	bool	first;

	first = true;

	// MAP_SHARED and MAP_PRIVATE are usually mutually exclusive
	if ((flags & MAP_SHARED) == MAP_SHARED)
	{
		fprintf(stderr, "MAP_SHARED");
		first = false;
	}
	else if ((flags & MAP_PRIVATE) == MAP_PRIVATE)
	{
		fprintf(stderr, "MAP_PRIVATE");
		first = false;
	}

	if (flags & MAP_FIXED)
		print_flag("MAP_FIXED", &first);
	if (flags & MAP_DROPPABLE)
		print_flag("MAP_DROPPABLE", &first);
	if (flags & MAP_ANONYMOUS)
		print_flag("MAP_ANONYMOUS", &first);
	if (flags & MAP_DENYWRITE)
		print_flag("MAP_DENYWRITE", &first);
	if (flags & MAP_STACK)
		print_flag("MAP_STACK", &first);
	if (flags & MAP_LOCKED)
		print_flag("MAP_LOCKED", &first);
	if (flags & MAP_NORESERVE)
		print_flag("MAP_NORESERVE", &first);
	if (flags & MAP_POPULATE)
		print_flag("MAP_POPULATE", &first);
}
