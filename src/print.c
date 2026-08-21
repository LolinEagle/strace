#include "ft_strace.h"

const char	*get_signal_name(int signo)
{
	static const char *const	sig_names[NSIG] = {
	[SIGHUP] = "SIGHUP",
	[SIGINT] = "SIGINT",
	[SIGQUIT] = "SIGQUIT",
	[SIGILL] = "SIGILL",
	[SIGTRAP] = "SIGTRAP",
	[SIGABRT] = "SIGABRT",
	[SIGBUS] = "SIGBUS",
	[SIGFPE] = "SIGFPE",
	[SIGKILL] = "SIGKILL",
	[SIGUSR1] = "SIGUSR1",
	[SIGSEGV] = "SIGSEGV",
	[SIGUSR2] = "SIGUSR2",
	[SIGPIPE] = "SIGPIPE",
	[SIGALRM] = "SIGALRM",
	[SIGTERM] = "SIGTERM",
	[SIGCHLD] = "SIGCHLD",
	[SIGCONT] = "SIGCONT",
	[SIGSTOP] = "SIGSTOP",
	[SIGTSTP] = "SIGTSTP",
	[SIGTTIN] = "SIGTTIN",
	[SIGTTOU] = "SIGTTOU",
	[SIGURG] = "SIGURG",
	[SIGXCPU] = "SIGXCPU",
	[SIGXFSZ] = "SIGXFSZ",
	[SIGVTALRM] = "SIGVTALRM",
	[SIGPROF] = "SIGPROF",
	[SIGWINCH] = "SIGWINCH",
	[SIGIO] = "SIGIO",
	[SIGPWR] = "SIGPWR",
	[SIGSYS] = "SIGSYS",
	};

	if (signo > 0 && signo < NSIG && sig_names[signo] != NULL)
		return (sig_names[signo]);

	return ("UNKNOWN");
}

char	*format(int i)
{
	if (i > 0)
		return (", ");
	else
		return ("");
}

void	print_syscall_entry(t_tracer *t)
{
	long			sys_no;
	const char		*name = NULL;
	int				nargs;
	unsigned long	args[6];
	int				i;

	nargs = 6;
	memset(args, 0, sizeof(args));
	if (t->arch == ARCH_64)
	{
		sys_no = t->regs64.orig_rax;
		t->orig_syscall = sys_no;
		if (sys_no >= 0 && (size_t)sys_no < g_syscalls_64_count
			&& g_syscalls_64[sys_no].name)
		{
			name = g_syscalls_64[sys_no].name;
			nargs = g_syscalls_64[sys_no].nargs;
		}
		args[0] = t->regs64.rdi;
		args[1] = t->regs64.rsi;
		args[2] = t->regs64.rdx;
		args[3] = t->regs64.r10;
		args[4] = t->regs64.r8;
		args[5] = t->regs64.r9;
	}
	else
	{
		sys_no = t->regs32.orig_eax;
		t->orig_syscall = sys_no;
		if (sys_no >= 0 && (size_t)sys_no < g_syscalls_32_count
			&& g_syscalls_32[sys_no].name)
		{
			name = g_syscalls_32[sys_no].name;
			nargs = g_syscalls_32[sys_no].nargs;
		}
		args[0] = t->regs32.ebx;
		args[1] = t->regs32.ecx;
		args[2] = t->regs32.edx;
		args[3] = t->regs32.esi;
		args[4] = t->regs32.edi;
		args[5] = t->regs32.ebp;
	}

	if (name)
		fprintf(stderr, "%s(", name);
	else
		fprintf(stderr, "sys_%ld(", t->orig_syscall);
	i = 0;
	while (i < nargs)
	{
		if (args[i] == 0)
			fprintf(stderr, "%s0", format(i));
		else
			fprintf(stderr, "%s0x%lx", format(i), args[i]);
		i++;
	}
	fprintf(stderr, ")");
	fflush(stderr);
}

void	print_syscall_exit(t_tracer *t)
{
	long	ret;

	if (t->arch == ARCH_64)
		ret = (long)t->regs64.rax;
	else
		ret = (long)(int32_t)t->regs32.eax;
	if (ret < 0 && ret >= -4095)
		fprintf(stderr, " = -1 (errno %ld)\n", -ret);
	else if (ret == 0)
		fprintf(stderr, " = 0\n");
	else if (ret > 0 && ret <= 4096)
		fprintf(stderr, " = %ld\n", ret);
	else
		fprintf(stderr, " = 0x%lx\n", ret);
	fflush(stderr);
}

void	print_signal(siginfo_t *si)
{
	fprintf(stderr, "--- %s {si_signo=%s, si_code=%d",
		get_signal_name(si->si_signo),
		get_signal_name(si->si_signo),
		si->si_code);
	if (si->si_signo == SIGSEGV || si->si_signo == SIGILL
		|| si->si_signo == SIGBUS || si->si_signo == SIGFPE)
		fprintf(stderr, ", si_addr=%p", si->si_addr);
	fprintf(stderr, "} ---\n");
	fflush(stderr);
}
