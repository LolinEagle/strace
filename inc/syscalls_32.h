#include "ft_strace.h"

// Linux x86 (i386) syscall table snippet
const t_syscall_entry	g_syscalls_32[] = {
[1] = {"exit", 1},
[2] = {"fork", 0},
[3] = {"read", 3},
[4] = {"write", 3},
[5] = {"open", 3},
[6] = {"close", 1},
[7] = {"waitpid", 3},
[8] = {"creat", 2},
[9] = {"link", 2},
[10] = {"unlink", 1},
[11] = {"execve", 3},
[12] = {"chdir", 1},
[13] = {"time", 1},
[14] = {"mknod", 3},
[15] = {"chmod", 2},
[19] = {"lseek", 3},
[20] = {"getpid", 0},
[33] = {"access", 2},
[37] = {"kill", 2},
[39] = {"mkdir", 2},
[40] = {"rmdir", 1},
[41] = {"dup", 1},
[42] = {"pipe", 1},
[45] = {"brk", 1},
[63] = {"dup2", 2},
[64] = {"getppid", 0},
[90] = {"old_mmap", 1},
[91] = {"munmap", 2},
[122] = {"uname", 1},
[125] = {"mprotect", 3},
[174] = {"rt_sigaction", 4},
[175] = {"rt_sigprocmask", 4},
[177] = {"rt_sigreturn", 0},
[192] = {"mmap2", 6},
[240] = {"futex", 6},
[252] = {"exit_group", 1},
[295] = {"openat", 4}
};
const size_t			g_syscalls_32_count = sizeof(g_syscalls_32)
	/ sizeof(g_syscalls_32[0]);
