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
	t_syscall_entry	e;
	unsigned long	args[6];
	int				i;

	e.nargs = 6;
	memset(args, 0, sizeof(args));
	if (t->arch == ARCH_64)
	{
		sys_no = t->regs64.orig_rax;
		t->orig_syscall = sys_no;
		if (sys_no >= 0 && (size_t)sys_no < g_syscalls_64_count
			&& g_syscalls_64[sys_no].name)
		{
			name = g_syscalls_64[sys_no].name;
			e.nargs = g_syscalls_64[sys_no].nargs;
			i = -1;
			while (++i < e.nargs)
				e.args_type[i] = g_syscalls_64[sys_no].args_type[i];
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
			e.nargs = g_syscalls_32[sys_no].nargs;
			i = -1;
			while (++i < e.nargs)
				e.args_type[i] = g_syscalls_32[sys_no].args_type[i];
		}
		args[0] = t->regs32.ebx;
		args[1] = t->regs32.ecx;
		args[2] = t->regs32.edx;
		args[3] = t->regs32.esi;
		args[4] = t->regs32.edi;
		args[5] = t->regs32.ebp;
	}

	if (name)
		fprintf(stderr, YELLOW "%s" RESET "(", name);
	else
		fprintf(stderr, YELLOW "sys_%ld" RESET "(", t->orig_syscall);
	i = 0;
	while (i < e.nargs)
	{
		fprintf(stderr, "%s" MAGENTA, format(i));
		if (e.args_type[i] == INT)
			fprintf(stderr, "%ld", (long)args[i]);
		else if (e.args_type[i] == UINT)
			fprintf(stderr, "%lu", (unsigned long)args[i]);
		else if (e.args_type[i] == HEX)
			fprintf(stderr, "0x%lx", (unsigned long)args[i]);
		else if (e.args_type[i] == PTR)
		{
			if (args[i] == 0)
				fprintf(stderr, "NULL");
			else
				fprintf(stderr, "0x%lx", (unsigned long)args[i]);
		}
		else if (e.args_type[i] == STR)
			print_syscall_entry_string(t->child_pid, args[i]);
		else if (e.args_type[i] == OCTAL)
			fprintf(stderr, "0%lo", (unsigned long)args[i]);
		else
			fprintf(stderr, "0x%lx", args[i]);
		fprintf(stderr, RESET);
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

	// Handle standard negative errno convention (-4095 to -1)
	if (ret < 0 && ret >= -4095)
		fprintf(stderr, " = " GREEN "-1 " RED "(errno %ld)\n" RESET, -ret);
	else if (ret == 0)
		fprintf(stderr, " = " GREEN "0\n" RESET);
	else if (t->arch == ARCH_32)
	{
		if ((uint32_t)ret > 10000)
			fprintf(stderr, " = " GREEN "0x%x\n" RESET, (uint32_t)ret);
		else
			fprintf(stderr, " = " GREEN "%u\n" RESET, (uint32_t)ret);
	}
	else
	{
		if ((unsigned long)ret > 10000)
			fprintf(stderr, " = " GREEN "0x%lx\n" RESET, (unsigned long)ret);
		else
			fprintf(stderr, " = " GREEN "%ld\n" RESET, ret);
	}
}

void	print_signal(siginfo_t *si)
{
	const char	*sig = get_signal_name(si->si_signo);

	fprintf(stderr, "--- %s {si_signo=%s, si_code=%d", sig, sig, si->si_code);
	if (si->si_signo == SIGSEGV || si->si_signo == SIGILL
		|| si->si_signo == SIGBUS || si->si_signo == SIGFPE)
		fprintf(stderr, ", si_addr=%p", si->si_addr);
	fprintf(stderr, "} ---\n");
	fflush(stderr);
}
