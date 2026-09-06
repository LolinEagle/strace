#!/usr/bin/env bash

# Colors
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
BOLD="\033[1m"
RESET="\033[0m"

FT_STRACE="./ft_strace"
REAL_STRACE="strace"
TMP_DIR="./ft_strace_tests"
PASSED=0
FAILED=0

# Clean & create workspace
mkdir -p "$TMP_DIR"
trap 'rm -rf "$TMP_DIR"' EXIT

echo -e "${BOLD}${CYAN}=== Building ft_strace ===${RESET}"
make -B > /dev/null
if [ ! -f "$FT_STRACE" ]; then
	echo -e "${RED}Error: ft_strace binary not found.${RESET}"
	exit 1
fi

echo -e "${BOLD}${CYAN}=== Preparing Test Binaries ===${RESET}"

# 1. Basic 64-bit C program
gcc "test/basic.c" -o "$TMP_DIR/basic"

# 2. 32-bit C program (if gcc -m32 is supported)
gcc -m32 "test/basic.c" -o "$TMP_DIR/basic32" 2>/dev/null
HAS_32BIT=$?

# 3. Crash / Signal SIGSEGV
gcc "test/segfault.c" -o "$TMP_DIR/segfault"
gcc -m32 "test/segfault.c" -o "$TMP_DIR/segfault32"

# 4. Signal Traps & Alarms (SIGALRM & custom handler)
gcc "test/sigalrm.c" -o "$TMP_DIR/sigalrm"
gcc -m32 "test/sigalrm.c" -o "$TMP_DIR/sigalrm32"

# 5. Signal SIGFPE (Floating Point Exception)
gcc "test/sigfpe.c" -o "$TMP_DIR/sigfpe"
gcc -m32 "test/sigfpe.c" -o "$TMP_DIR/sigfpe32"

# Helper function to normalize syscall names & exit lines for robust matching
# (Extracts syscall names, signals, and exit/killed summaries)
normalize_trace(){
	local input="$1"
	grep -E '^[a-zA-Z0-9_]+\(|^--- SIG|\+\+\+ (exited|killed)' "$input" \
		| sed -E 's/\(.*//g' \
		| sed -E 's/.*(--- SIG[A-Z0-9]+).*/\1/'
}

run_test(){
	local title="$1"
	shift
	local cmd=("$@")

	local ft_out="$TMP_DIR/ft_out.txt"
	local ft_err="$TMP_DIR/ft_err.txt"
	local real_out="$TMP_DIR/real_out.txt"
	local real_err="$TMP_DIR/real_err.txt"

	# Run ft_strace
	"$FT_STRACE" "${cmd[@]}" > "$ft_out" 2> "$ft_err"
	local ft_ret=$?

	# Run real strace
	"$REAL_STRACE" "${cmd[@]}" > "$real_out" 2> "$real_err"
	local real_ret=$?

	# Extract normalized traces
	normalize_trace "$ft_err" > "$TMP_DIR/ft_norm.txt"
	normalize_trace "$real_err" > "$TMP_DIR/real_norm.txt"

	# Compare exit status line
	local ft_exit_summary
	ft_exit_summary=$(grep -E '^\+\+\+ (exited|killed)' "$ft_err" | tail -n1)
	local real_exit_summary
	real_exit_summary=$(grep -E '^\+\+\+ (exited|killed)' "$real_err" | tail -n1)

	echo -ne "[TEST] $title ... "

	local test_fail=0
	
	# 1. Compare return status of the command itself
	if [ "$ft_ret" -ne "$real_ret" ]; then
		test_fail=1
	fi

	# 2. Check exit summaries
	if [ "$ft_exit_summary" != "$real_exit_summary" ]; then
		test_fail=1
	fi

	# 3. Check that ft_strace captured syscalls (non-empty output)
	if [ ! -s "$TMP_DIR/ft_norm.txt" ]; then
		test_fail=1
	fi

	if [ "$test_fail" -eq 0 ]; then
		echo -e "${GREEN}${BOLD}[PASS]${RESET}"
		((PASSED++))
	else
		echo -e "${RED}${BOLD}[FAIL]${RESET}"
		((FAILED++))
		echo -e "${YELLOW}--- ft_strace output snippet ---${RESET}"
		tail -n 10 "$ft_err"
		echo -e "${YELLOW}--- real strace output snippet ---${RESET}"
		tail -n 10 "$real_err"
		echo "----------------------------------------"
	fi
}

echo -e "\n${BOLD}${CYAN}=== Running Test Cases ===${RESET}\n"

# Standard 64-bit commands
run_test "Standard /bin/echo" /bin/echo "42 Network"
run_test "Standard /bin/ls -la" /bin/ls -la "$TMP_DIR"
run_test "Standard custom 64-bit binary" "$TMP_DIR/basic"

# Exit Codes
run_test "Command returning non-zero (false)" /bin/false
run_test "Exit code propagation (sh -c 'exit 77')" /bin/sh -c "exit 77"

# Signals
run_test "SIGSEGV (Segmentation Fault)" "$TMP_DIR/segfault"
run_test "SIGFPE (Division by zero)" "$TMP_DIR/sigfpe"
run_test "SIGALRM & pause" "$TMP_DIR/sigalrm"

# 32-bit Architecture Support
if [ "$HAS_32BIT" -eq 0 ]; then
	run_test "32-bit binary execution" "$TMP_DIR/basic32"
	run_test "SIGSEGV (Segmentation Fault)" "$TMP_DIR/segfault32"
	run_test "SIGFPE (Division by zero)" "$TMP_DIR/sigfpe32"
	run_test "SIGALRM & pause" "$TMP_DIR/sigalrm32"
else
	echo -e "${YELLOW}[SKIP] 32-bit test skipped (gcc-multilib not installed).${RESET}"
fi

make fclean

# Summary
echo -e "\n${BOLD}${CYAN}=== Summary ===${RESET}"
echo -e "Total Passed: ${GREEN}${BOLD}${PASSED}${RESET}"
echo -e "Total Failed: ${RED}${BOLD}${FAILED}${RESET}"

if [ "$FAILED" -eq 0 ]; then
	echo -e "${GREEN}${BOLD}All tests passed successfully!${RESET}"
	exit 0
else
	echo -e "${RED}${BOLD}Some tests failed.${RESET}"
	exit 1
fi
