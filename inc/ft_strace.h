#pragma once

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <linux/ptrace.h>
#include <elf.h>

typedef enum s_arch
{
	ARCH_64,
	ARCH_32
}	t_arch;

typedef struct s_syscall_entry
{
	const char	*name;
	int			nargs;
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
	pid_t						child_pid;
	bool						in_syscall;
	t_arch						arch;
	struct user_regs_struct		regs64;
	t_user_regs_struct32		regs32;
	long						orig_syscall;
}	t_tracer;

// System call tables
extern const t_syscall_entry	g_syscalls_64[];
extern const size_t				g_syscalls_64_count;
extern const t_syscall_entry	g_syscalls_32[];
extern const size_t				g_syscalls_32_count;

// Prototypes
int			run_tracer(pid_t child_pid);
void		print_syscall_entry(t_tracer *t);
void		print_syscall_exit(t_tracer *t);
void		print_signal(siginfo_t *si);
const char	*get_signal_name(int signo);
