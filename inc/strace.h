#pragma once

#define _GNU_SOURCE
#define MAX_STR_LEN 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/uio.h>
#include <elf.h>
#include <ctype.h>

#include "colors_ansi.h"

typedef enum s_arch
{
	ARCH_64,
	ARCH_32
}	t_arch;

typedef enum s_arg_type
{
	NONE,
	INT,	// Decimal signed int: 1, -1
	UINT,	// Decimal unsigned int / size_t: 6
	HEX,	// Hexadecimal (flags/masks): 0x77
	PTR,	// Generic pointer: 0x7ffd1234 or NULL
	STR,	// NUL-terminated string: "Hello\n"
	OCTAL,	// Octal
	ARGV,
	ARG_TYPE_NBR
}	t_arg_type;

typedef struct s_syscall_entry
{
	const char	*name;
	int			nargs;
	t_arg_type	ret_type;
	t_arg_type	args_type[6];
}	t_syscall_entry;

// 32-bit register structure for x86 compat mode on x86_64
typedef struct s_user_regs_struct32
{
	uint32_t	ebx;
	uint32_t	ecx;
	uint32_t	edx;
	uint32_t	esi;
	uint32_t	edi;
	uint32_t	ebp;
	uint32_t	eax;
	uint32_t	xds;
	uint32_t	xes;
	uint32_t	xfs;
	uint32_t	xgs;
	uint32_t	orig_eax;
	uint32_t	eip;
	uint32_t	xcs;
	uint32_t	eflags;
	uint32_t	esp;
	uint32_t	xss;
}	t_user_regs_struct32;

typedef struct s_tracer
{
	int						argc;
	char					**argv;
	char					**envp;
	pid_t					child_pid;
	bool					in_syscall;
	t_arch					arch;
	struct user_regs_struct	regs64;
	t_user_regs_struct32	regs32;
	long					orig_syscall;
}	t_tracer;

// System call tables
extern const t_syscall_entry	g_syscalls_64[];
extern const size_t				g_syscalls_64_count;
extern const t_syscall_entry	g_syscalls_32[];
extern const size_t				g_syscalls_32_count;

// Prototypes
int			run_tracer(pid_t child_pid, int argc, char **argv, char **envp);

// Print getter
const char	*get_signal_name(int signo);
const char	*get_si_codes(int si_code, int si_signo);

// Print
void		print_syscall_entry(t_tracer *t);
void		print_syscall_exit(t_tracer *t);
void		print_signal(t_tracer *t, siginfo_t *si);

// Read string
void		print_syscall_entry_string(pid_t child_pid, unsigned long args,
				size_t count);
void		print_syscall_entry_argv(t_tracer *t);
