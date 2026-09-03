#include "ft_strace.h"

// Linux x86 (i386) syscall table snippet
const t_syscall_entry	g_syscalls_32[] = {
[1] = {"exit", 1, INT, {INT}},
[2] = {"fork", 0, INT, {}},
[3] = {"read", 3, INT, {INT, PTR, UINT}},
[4] = {"write", 3, INT, {INT, STR, UINT}},
[5] = {"open", 3, INT, {STR, HEX, OCTAL}},
[6] = {"close", 1, INT, {INT}},
[7] = {"waitpid", 3, INT, {INT, PTR, HEX}},
[8] = {"creat", 2, INT, {STR, OCTAL}},
[9] = {"link", 2, INT, {STR, STR}},
[10] = {"unlink", 1, INT, {STR}},
[11] = {"execve", 3, INT, {STR, PTR, PTR}},
[12] = {"chdir", 1, INT, {STR}},
[13] = {"time", 1, INT, {PTR}},
[14] = {"mknod", 3, INT, {STR, OCTAL, UINT}},
[15] = {"chmod", 2, INT, {STR, OCTAL}},
[19] = {"lseek", 3, INT, {INT, INT, INT}},
[20] = {"getpid", 0, INT, {}},
[33] = {"access", 2, INT, {STR, OCTAL}},
[37] = {"kill", 2, INT, {INT, INT}},
[39] = {"mkdir", 2, INT, {STR, OCTAL}},
[40] = {"rmdir", 1, INT, {STR}},
[41] = {"dup", 1, INT, {INT}},
[42] = {"pipe", 1, INT, {PTR}},
[45] = {"brk", 1, PTR, {PTR}},
[63] = {"dup2", 2, INT, {INT, INT}},
[64] = {"getppid", 0, INT, {}},
[90] = {"old_mmap", 1, PTR, {PTR}},
[91] = {"munmap", 2, INT, {PTR, UINT}},
[122] = {"uname", 1, INT, {PTR}},
[125] = {"mprotect", 3, INT, {PTR, UINT, HEX}},
[174] = {"rt_sigaction", 4, INT, {INT, PTR, PTR, UINT}},
[175] = {"rt_sigprocmask", 4, INT, {HEX, PTR, PTR, UINT}},
[177] = {"rt_sigreturn", 0, INT, {}},
[192] = {"mmap2", 6, PTR, {PTR, UINT, HEX, HEX, INT, UINT}},
[240] = {"futex", 6, INT, {PTR, HEX, INT, PTR, PTR, INT}},
[252] = {"exit_group", 1, INT, {INT}},
[295] = {"openat", 4, INT, {INT, STR, HEX, OCTAL}}
};
const size_t			g_syscalls_32_count = sizeof(g_syscalls_32)
	/ sizeof(g_syscalls_32[0]);
