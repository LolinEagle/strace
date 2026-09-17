#include "strace.h"

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

const char	*get_si_codes(int si_code, int si_signo)
{
	static const char *const	sigfpe[16] = {
	[FPE_INTDIV] = "FPE_INTDIV",		// integer divide by zero
	[FPE_INTOVF] = "FPE_INTOVF",		// integer overflow
	[FPE_FLTDIV] = "FPE_FLTDIV",		// floating point divide by zero
	[FPE_FLTOVF] = "FPE_FLTOVF",		// floating point overflow
	[FPE_FLTUND] = "FPE_FLTUND",		// floating point underflow
	[FPE_FLTRES] = "FPE_FLTRES",		// floating point inexact result
	[FPE_FLTINV] = "FPE_FLTINV",		// floating point invalid operation
	[FPE_FLTSUB] = "FPE_FLTSUB",		// subscript out of range
	[9] = "__FPE_DECOVF",				// decimal overflow
	[10] = "__FPE_DECDIV",				// decimal division by zero
	[11] = "__FPE_DECERR",				// packed decimal error
	[12] = "__FPE_INVASC",				// invalid ASCII digit
	[13] = "__FPE_INVDEC",				// invalid decimal digit
	[FPE_FLTUNK] = "FPE_FLTUNK",		// undiagnosed floating-point exception
	[FPE_CONDTRAP] = "FPE_CONDTRAP",	// trap on condition
	};
	static const char *const	sigsegv[10] = {
	[SEGV_MAPERR] = "SEGV_MAPERR",		// address not mapped to object
	[SEGV_ACCERR] = "SEGV_ACCERR",		// invalid permissions for mapped object
	[SEGV_BNDERR] = "SEGV_BNDERR",		// failed address bound checks
	[SEGV_PKUERR] = "SEGV_PKUERR",		// failed protection key checks
	[SEGV_ACCADI] = "SEGV_ACCADI",		// ADI not enabled for mapped object
	[SEGV_ADIDERR] = "SEGV_ADIDERR",	// Disrupting MCD error
	[SEGV_ADIPERR] = "SEGV_ADIPERR",	// Precise MCD exception
	[SEGV_MTEAERR] = "SEGV_MTEAERR",	// Asynchronous ARM MTE error
	[SEGV_MTESERR] = "SEGV_MTESERR",	// Synchronous ARM MTE exception
	};
	static char					str[4];

	if (si_signo == SIGINT || si_signo == SIGALRM || si_signo == SIGTERM)
	{
		if (si_code == 0)
			return ("SI_USER");// sent by kill, sigsend, raise
		else if (si_code == 128)
			return ("SI_KERNEL");// sent by the kernel from somewhere
		else if (si_code == -1)
			return ("SI_QUEUE");// sent by sigqueue
		else if (si_code == -2)
			return ("SI_TIMER");// sent by timer expiration
		else if (si_code == -3)
			return ("SI_MESGQ");// sent by real time mesq state change
		else if (si_code == -4)
			return ("SI_ASYNCIO");// sent by AIO completion
		else if (si_code == -5)
			return ("SI_SIGIO");// sent by queued SIGIO
		else if (si_code == -6)
			return ("SI_TKILL");// sent by tkill system call
		else if (si_code == -7)
			return ("SI_DETHREAD");// sent by execve killing subsidiary threads
		else if (si_code == -60)
			return ("SI_ASYNCNL");// sent by glibc async name lookup completion
	}
	else if (si_signo == SIGFPE)
	{
		if (si_code > 0 && si_code < 10 && sigfpe[si_code] != NULL)
			return (sigfpe[si_code]);
	}
	else if (si_signo == SIGSEGV)
	{
		if (si_code > 0 && si_code < 10 && sigsegv[si_code] != NULL)
			return (sigsegv[si_code]);
	}
	sprintf(str, "%d", si_code);
	return (str);
}
