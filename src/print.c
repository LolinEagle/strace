#include "strace.h"

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
		else
		{
			i = -1;
			while (++i < 6)
				e.args_type[i] = NONE;
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
		else
		{
			i = -1;
			while (++i < 6)
				e.args_type[i] = NONE;
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

	// Argument
	i = 0;
	while (i < e.nargs)
	{
		fprintf(stderr, RESET "%s" MAGENTA, format(i));
		if (e.args_type[i] == INT)
			fprintf(stderr, "%i", (int)args[i]);
		else if (e.args_type[i] == UINT)
			fprintf(stderr, "%lu", (unsigned long)args[i]);
		else if (e.args_type[i] == HEX)
		{
			if (args[i] == 0)
				fprintf(stderr, "0");
			else
				fprintf(stderr, "0x%lx", (unsigned long)args[i]);
		}
		else if (e.args_type[i] == PTR)
		{
			if (args[i] == 0)
				fprintf(stderr, "NULL");
			else
				fprintf(stderr, "0x%lx", (unsigned long)args[i]);
		}
		else if (e.args_type[i] == STR)
		{
			if (name && (strcmp(name, "read") == 0
					|| strcmp(name, "pread64") == 0))
				print_syscall_entry_string(t->child_pid, args[i], args[i + 1]);
			else
				print_syscall_entry_string(t->child_pid, args[i], ULONG_MAX);
		}
		else if (e.args_type[i] == OCTAL)
			fprintf(stderr, "0%lo", (unsigned long)args[i]);
		else if (e.args_type[i] == ARGV)
			print_syscall_entry_argv(t);
		else if (e.args_type[i] == PROT)
			decode_mmap_prot(args[i]);
		else if (e.args_type[i] == MMAP_FLAGS)
			decode_mmap_flags(args[i]);
		else if (e.args_type[i] == MODE)
			decode_access_mode(args[i]);
		else if (e.args_type[i] == DIRFD)
			decode_openat_dirfd(args[i]);
		else if (e.args_type[i] == STRUCT)
			fprintf(stderr, "{0x%lx}", (unsigned long)args[i]);
		else if (e.args_type[i] == OPENAT_FLAGS)
			decode_openat_flags(args[i]);
		else if (e.args_type[i] == OP)
			decode_arch_prctl_op(args[i]);
		else if (e.args_type[i] == RESOURCE)
			decode_prlimit64_resource(args[i]);
		else if (e.args_type[i] == GETRANDOM_FLAGS)
			decode_getrandom_flags(args[i]);
		else
			fprintf(stderr, "0x%lx", args[i]);
		i++;
	}

	// Execve exception
	if (name && strcmp(name, "execve") == 0)
	{
		i = 0;
		while (t->envp[i] != NULL)
			i++;
		fprintf(stderr, CYAN " /* %i vars */", i);
	}

	// Flush
	fflush(stderr);
}

void	print_syscall_exit(t_tracer *t)
{
	long						ret;
	static const char *const	errno_names[134] = {
	[1] = "EPERM (Operation not permitted)\n",
	[2] = "ENOENT (No such file or directory)\n",
	[3] = "ESRCH (No such process)\n",
	[4] = "EINTR (Interrupted system call)\n",
	[5] = "EIO (Input/output error)\n",
	[6] = "ENXIO (No such device or address)\n",
	[7] = "E2BIG (Argument list too long)\n",
	[8] = "ENOEXEC (Exec format error)\n",
	[9] = "EBADF (Bad file descriptor)\n",
	[10] = "ECHILD (No child processes)\n",
	[11] = "EAGAIN (Resource temporarily unavailable)\n",
	[12] = "ENOMEM (Cannot allocate memory)\n",
	[13] = "EACCES (Permission denied)\n",
	[14] = "EFAULT (Bad address)\n",
	[15] = "ENOTBLK (Block device required)\n",
	[16] = "EBUSY (Device or resource busy)\n",
	[17] = "EEXIST (File exists)\n",
	[18] = "EXDEV (Invalid cross-device link)\n",
	[19] = "ENODEV (No such device)\n",
	[20] = "ENOTDIR (Not a directory)\n",
	[21] = "EISDIR (Is a directory)\n",
	[22] = "EINVAL (Invalid argument)\n",
	[23] = "ENFILE (Too many open files in system)\n",
	[24] = "EMFILE (Too many open files)\n",
	[25] = "ENOTTY (Inappropriate ioctl for device)\n",
	[26] = "ETXTBSY (Text file busy)\n",
	[27] = "EFBIG (File too large)\n",
	[28] = "ENOSPC (No space left on device)\n",
	[29] = "ESPIPE (Illegal seek)\n",
	[30] = "EROFS (Read-only file system)\n",
	[31] = "EMLINK (Too many links)\n",
	[32] = "EPIPE (Broken pipe)\n",
	[33] = "EDOM (Numerical argument out of domain)\n",
	[34] = "ERANGE (Numerical result out of range)\n",
	[35] = "EDEADLK (Resource deadlock avoided)\n",
	[36] = "ENAMETOOLONG (File name too long)\n",
	[37] = "ENOLCK (No locks available)\n",
	[38] = "ENOSYS (Function not implemented)\n",
	[39] = "ENOTEMPTY (Directory not empty)\n",
	[40] = "ELOOP (Too many levels of symbolic links)\n",
	[41] = " (not implemented)\n",
	[42] = "ENOMSG (No message of desired type)\n",
	[43] = "EIDRM (Identifier removed)\n",
	[44] = "ECHRNG (Channel number out of range)\n",
	[45] = "EL2NSYNC (Level 2 not synchronized)\n",
	[46] = "EL3HLT (Level 3 halted)\n",
	[47] = "EL3RST (Level 3 reset)\n",
	[48] = "ELNRNG (Link number out of range)\n",
	[49] = "EUNATCH (Protocol driver not attached)\n",
	[50] = "ENOCSI (No CSI structure available)\n",
	[51] = "EL2HLT (Level 2 halted)\n",
	[52] = "EBADE (Invalid exchange)\n",
	[53] = "EBADR (Invalid request descriptor)\n",
	[54] = "EXFULL (Exchange full)\n",
	[55] = "ENOANO (No anode)\n",
	[56] = "EBADRQC (Invalid request code)\n",
	[57] = "EBADSLT (Invalid slot)\n",
	[58] = " (not implemented)\n",
	[59] = "EBFONT (Bad font file format)\n",
	[60] = "ENOSTR (Device not a stream)\n",
	[61] = "ENODATA (No data available)\n",
	[62] = "ETIME (Timer expired)\n",
	[63] = "ENOSR (Out of streams resources)\n",
	[64] = "ENONET (Machine is not on the network)\n",
	[65] = "ENOPKG (Package not installed)\n",
	[66] = "EREMOTE (Object is remote)\n",
	[67] = "ENOLINK (Link has been severed)\n",
	[68] = "EADV (Advertise error)\n",
	[69] = "ESRMNT (Srmount error)\n",
	[70] = "ECOMM (Communication error on send)\n",
	[71] = "EPROTO (Protocol error)\n",
	[72] = "EMULTIHOP (Multihop attempted)\n",
	[73] = "EDOTDOT (RFS specific error)\n",
	[74] = "EBADMSG (Bad message)\n",
	[75] = "EOVERFLOW (Value too large for defined data type)\n",
	[76] = "ENOTUNIQ (Name not unique on network)\n",
	[77] = "EBADFD (File descriptor in bad state)\n",
	[78] = "EREMCHG (Remote address changed)\n",
	[79] = "ELIBACC (Can not access a needed shared library)\n",
	[80] = "ELIBBAD (Accessing a corrupted shared library)\n",
	[81] = "ELIBSCN (.lib section in a.out corrupted)\n",
	[82] = "ELIBMAX (Attempting to link in too many shared libraries)\n",
	[83] = "ELIBEXEC (Cannot exec a shared library directly)\n",
	[84] = "EILSEQ (Invalid or incomplete multibyte or wide character)\n",
	[85] = "ERESTART (Interrupted system call should be restarted)\n",
	[86] = "ESTRPIPE (Streams pipe error)\n",
	[87] = "EUSERS (Too many users)\n",
	[88] = "ENOTSOCK (Socket operation on non-socket)\n",
	[89] = "EDESTADDRREQ (Destination address required)\n",
	[90] = "EMSGSIZE (Message too long)\n",
	[91] = "EPROTOTYPE (Protocol wrong type for socket)\n",
	[92] = "ENOPROTOOPT (Protocol not available)\n",
	[93] = "EPROTONOSUPPORT (Protocol not supported)\n",
	[94] = "ESOCKTNOSUPPORT (Socket type not supported)\n",
	[95] = "EOPNOTSUPP (Operation not supported)\n",
	[96] = "EPFNOSUPPORT (Protocol family not supported)\n",
	[97] = "EAFNOSUPPORT (Address family not supported by protocol)\n",
	[98] = "EADDRINUSE (Address already in use)\n",
	[99] = "EADDRNOTAVAIL (Cannot assign requested address)\n",
	[100] = "ENETDOWN (Network is down)\n",
	[101] = "ENETUNREACH (Network is unreachable)\n",
	[102] = "ENETRESET (Network dropped connection on reset)\n",
	[103] = "ECONNABORTED (Software caused connection abort)\n",
	[104] = "ECONNRESET (Connection reset by peer)\n",
	[105] = "ENOBUFS (No buffer space available)\n",
	[106] = "EISCONN (Transport endpoint is already connected)\n",
	[107] = "ENOTCONN (Transport endpoint is not connected)\n",
	[108] = "ESHUTDOWN (Cannot send after transport endpoint shutdown)\n",
	[109] = "ETOOMANYREFS (Too many references: cannot splice)\n",
	[110] = "ETIMEDOUT (Connection timed out)\n",
	[111] = "ECONNREFUSED (Connection refused)\n",
	[112] = "EHOSTDOWN (Host is down)\n",
	[113] = "EHOSTUNREACH (No route to host)\n",
	[114] = "EALREADY (Operation already in progress)\n",
	[115] = "EINPROGRESS (Operation now in progress)\n",
	[116] = "ESTALE (Stale file handle)\n",
	[117] = "EUCLEAN (Structure needs cleaning)\n",
	[118] = "ENOTNAM (Not a XENIX named type file)\n",
	[119] = "ENAVAIL (No XENIX semaphores available)\n",
	[120] = "EISNAM (Is a named type file)\n",
	[121] = "EREMOTEIO (Remote I/O error)\n",
	[122] = "EDQUOT (Disk quota exceeded)\n",
	[123] = "ENOMEDIUM (No medium found)\n",
	[124] = "EMEDIUMTYPE (Wrong medium type)\n",
	[125] = "ECANCELED (Operation canceled)\n",
	[126] = "ENOKEY (Required key not available)\n",
	[127] = "EKEYEXPIRED (Key has expired)\n",
	[128] = "EKEYREVOKED (Key has been revoked)\n",
	[129] = "EKEYREJECTED (Key was rejected by service)\n",
	[130] = "EOWNERDEAD (Owner died)\n",
	[131] = "ENOTRECOVERABLE (State not recoverable)\n",
	[132] = "ERFKILL (Operation not possible due to RF-kill)\n",
	[133] = "EHWPOISON (Memory page has hardware error)\n"
	};
	const char					*name = NULL;
	t_arg_type					ret_type;
	long						sys_no;

	if (t->arch == ARCH_64)
	{
		ret = (long)t->regs64.rax;
		sys_no = t->regs64.orig_rax;
		t->orig_syscall = sys_no;
		if (sys_no >= 0 && (size_t)sys_no < g_syscalls_64_count
			&& g_syscalls_64[sys_no].name)
		{
			name = g_syscalls_64[sys_no].name;
			ret_type = g_syscalls_64[sys_no].ret_type;
		}
	}
	else
	{
		ret = (long)(int32_t)t->regs32.eax;
		sys_no = t->regs32.orig_eax;
		t->orig_syscall = sys_no;
		if (sys_no >= 0 && (size_t)sys_no < g_syscalls_32_count
			&& g_syscalls_32[sys_no].name)
		{
			name = g_syscalls_32[sys_no].name;
			ret_type = g_syscalls_32[sys_no].ret_type;
		}
	}

	fprintf(stderr, RESET ")");
	if (ret == -516)
		fprintf(stderr, " = " GREEN "? " RED
			"ERESTART_RESTARTBLOCK (Interrupted by signal)\n" RESET);
	else if (ret == -514)
		fprintf(stderr, " = " GREEN "? " RED
			"ERESTARTNOHAND (To be restarted if no handler)\n" RESET);
	else if (ret <= -1 && ret >= -133)
		fprintf(stderr, " = " GREEN "-1 " RED "%s" RESET, errno_names[-ret]);
	else if (ret < 0 && ret >= -4095)
		fprintf(stderr, " = " GREEN "-1 " RED "(errno %ld)\n" RESET, -ret);
	else if (ret == 0)
	{
		fprintf(stderr, " = " GREEN "0");
		if (name && strcmp(name, "execve") == 0 && t->arch != ARCH_64)
			fprintf(stderr, "\n[ Process PID=%i runs in 32 bit mode. ]\n" RESET,
				t->child_pid);
		else if (name && strcmp(name, "poll") == 0)
			fprintf(stderr, " (Timeout)\n" RESET);
		else if (name && strcmp(name, "set_thread_area") == 0)
			fprintf(stderr, " (entry_number=12)\n" RESET);
		else
			fprintf(stderr, "\n" RESET);
	}
	else if (t->arch == ARCH_32)
	{
		if (ret_type == PTR)
			fprintf(stderr, " = " GREEN "0x%x\n" RESET, (uint32_t)ret);
		else if (name && strcmp(name, "fcntl") == 0 && ret == 524290)
			fprintf(stderr, " = " GREEN "0x%x (flags O_RDWR|O_CLOEXEC)\n"
				RESET, (uint32_t)ret);
		else
			fprintf(stderr, " = " GREEN "%u\n" RESET, (uint32_t)ret);
	}
	else
	{
		if (ret_type == PTR)
			fprintf(stderr, " = " GREEN "0x%lx\n" RESET, (unsigned long)ret);
		else if (name && strcmp(name, "fcntl") == 0 && ret == 524290)
			fprintf(stderr, " = " GREEN "0x%lx (flags O_RDWR|O_CLOEXEC)\n"
				RESET, (unsigned long)ret);
		else
			fprintf(stderr, " = " GREEN "%ld\n" RESET, ret);
	}
}

void	print_signal(t_tracer *t, siginfo_t *si)
{
	const char	*sig = get_signal_name(si->si_signo);

	fprintf(stderr, "--- %s {si_signo=" MAGENTA "%s" RESET ", si_code=" BLUE
		"%s" RESET, sig, sig, get_si_codes(si->si_code, si->si_signo));
	if (si->si_signo == SIGSEGV || si->si_signo == SIGILL
		|| si->si_signo == SIGBUS || si->si_signo == SIGFPE)
	{
		if (si->si_addr == 0)
			fprintf(stderr, ", si_addr=" MAGENTA "NULL" RESET);
		else
			fprintf(stderr, ", si_addr=" MAGENTA "%p" RESET, si->si_addr);
	}
	else if (si->si_signo == SIGINT || si->si_signo == SIGTERM
		|| si->si_signo == SIGWINCH)
	{
		fprintf(stderr, ", si_pid=" MAGENTA "%i" RESET ", si_uid=" MAGENTA
			"%i" RESET, t->child_pid, si->si_uid);
	}
	else if (si->si_signo == SIGCHLD)
	{
		fprintf(stderr, ", si_pid=" MAGENTA "%i" RESET ", si_uid=" MAGENTA
			"%i" RESET ", si_status=" MAGENTA "%i" RESET ", si_utime=" MAGENTA
			"0" RESET ", si_stime=" MAGENTA "0" RESET, t->child_pid, si->si_uid,
			si->si_status);
	}
	fprintf(stderr, "} ---\n");
	fflush(stderr);
}
