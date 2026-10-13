# Strace
A lightweight implementation of the Linux `strace` command. This tool traces system calls and signals received by a process.

## Features
- **Cross-Architecture**: Supports tracing both 64-bit (x86_64) and 32-bit (i386) binaries.
- **Restricted API**: Built using only a strictly allowed subset of `ptrace` requests (`PTRACE_SYSCALL`, `PTRACE_GETREGSET`, `PTRACE_SEIZE`, `PTRACE_GETSIGINFO`, etc.).
- **Signal Handling**: Accurately intercepts, logs, and forwards signals to the traced process.
- **Robust Execution**: Carefully manages execution state and unexpected child terminations without crashing.

## Usage
Run `ft_strace` followed by the command you want to trace (and its arguments):

```bash
./ft_strace <command> [arguments...]
```

**Example:**
```bash
./ft_strace ls -l
```
