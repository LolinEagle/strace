#!/usr/bin/env bash

# Colors
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
RESET="\033[0m"

MY_STRACE="./ft_strace"
REAL_STRACE="strace"
TMP_DIR="./strace_tests"
PASSED=0
FAILED=0
DIFF=0
TEST_NBR=0

# Clean & create workspace
mkdir -p "$TMP_DIR"
trap 'rm -rf "$TMP_DIR"' EXIT

make -B > /dev/null
if [ ! -f "$MY_STRACE" ]; then
	echo -e "${RED}Error: binary not found.${RESET}"
	exit 1
fi

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

export LANG=C

normalize_trace(){
	local input="$1"

	# Removing Arguments inside Parentheses
	# Standardizing Spacing Around =
	# Normalizing Memory/Hex Addresses
	# Normalizing Thread/Process ID Return Values
	# Normalizing Signal Sender PIDs
	sed -E \
		-e 's/\(.*\)([[:space:]]*=)/\(\)\1/g' \
		-e 's/\)[[:space:]]*=/) =/g' \
		-e 's/0x[0-9a-fA-F]+/0x[ADDR]/g' \
		-e 's/^(set_tid_address|gettid|getpid|getppid|clone|wait4|epoll_pwait2)\(\)[[:space:]]*=[[:space:]]*[0-9]+/\1() = [TID]/g' \
		-e 's/(PID|si_pid)=[0-9]+/si_pid=[PID]/g' \
		"$input"
}

run_test(){
	local title="$1"
	shift
	local cmd=("$@")

	((TEST_NBR++))
	mkdir -p "$TMP_DIR/$TEST_NBR"

	local my_out="$TMP_DIR/$TEST_NBR/my_out.txt"
	local my_err="$TMP_DIR/$TEST_NBR/my_err.txt"
	local real_out="$TMP_DIR/$TEST_NBR/real_out.txt"
	local real_err="$TMP_DIR/$TEST_NBR/real_err.txt"

	# Run
	"$MY_STRACE" "${cmd[@]}" > "$my_out" 2> "$my_err"
	local my_ret=$?
	"$REAL_STRACE" "${cmd[@]}" > "$real_out" 2> "$real_err"
	local real_ret=$?

	# Extract normalized traces
	normalize_trace "$my_err" > "$TMP_DIR/$TEST_NBR/my_norm.txt"
	normalize_trace "$real_err" > "$TMP_DIR/$TEST_NBR/real_norm.txt"

	# Compare exit status line
	local my_exit_summary
	my_exit_summary=$(grep -E '^\+\+\+ (exited|killed)' "$my_err" | tail -n1)
	local real_exit_summary
	real_exit_summary=$(grep -E '^\+\+\+ (exited|killed)' "$real_err" | tail -n1)

	echo -ne "[TEST] $TEST_NBR $title\n"

	local test_fail=0
	
	# 1. Compare return status of the command itself
	if [ "$my_ret" -ne "$real_ret" ]; then
		test_fail=1
	fi

	# 2. Check exit summaries
	if [ "$my_exit_summary" != "$real_exit_summary" ]; then
		test_fail=1
	fi

	# 3. Check that captured syscalls (non-empty output)
	if [ ! -s "$TMP_DIR/$TEST_NBR/my_norm.txt" ]; then
		test_fail=1
	fi

	# Compare the normalized outputs directly
	if diff -u0 "$TMP_DIR/$TEST_NBR/real_norm.txt" "$TMP_DIR/$TEST_NBR/my_norm.txt" > "$TMP_DIR/$TEST_NBR/diff.txt"; then
		echo -e "${GREEN}[DIFF] No diff found${RESET}"
	else
		echo -e "${YELLOW}[DIFF] Diff found :${RESET}"
		cat $TMP_DIR/$TEST_NBR/diff.txt
		((DIFF++))
	fi

	if [ "$test_fail" -eq 0 ]; then
		echo -e "${GREEN}[PASS]${RESET}"
		((PASSED++))
	else
		echo -e "${RED}[FAIL]${RESET}"
		((FAILED++))
		echo -e "${YELLOW}--- output snippet ---${RESET}"
		tail -n 3 "$my_err"
		echo -e "${YELLOW}--- real output snippet ---${RESET}"
		tail -n 3 "$real_err"
		echo "----------------------------------------"
	fi
}

echo -e "${CYAN}Running Test Cases :${RESET}"

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

# Standard POSIX CLI Utilities
run_test "Standard CLI: /bin/uname -a" /bin/uname -a
run_test "Standard CLI: /bin/pwd" /bin/pwd
run_test "Standard CLI: /bin/cat /dev/null" /bin/cat /dev/null
run_test "Standard CLI: /bin/sleep 0.1" /bin/sleep 0.1
run_test "Standard CLI: /bin/date" /bin/date

# Advanced Signal Handlers & Traps
run_test "Signal Self-kill (kill(getpid(), SIGTERM))" /bin/sh -c "kill -TERM \$\$"
run_test "Signal Self-kill (kill(getpid(), SIGINT))" /bin/sh -c "kill -INT \$\$"

# Argument & Execution Boundaries
run_test "Non-existent Command Error Handling" /bin/this_command_does_not_exist

# 32-bit Architecture Support
if [ "$HAS_32BIT" -eq 0 ]; then
	run_test "32-bit binary execution" "$TMP_DIR/basic32"
	run_test "SIGSEGV (Segmentation Fault)" "$TMP_DIR/segfault32"
	run_test "SIGFPE (Division by zero)" "$TMP_DIR/sigfpe32"
	run_test "SIGALRM & pause" "$TMP_DIR/sigalrm32"
else
	echo -e "${YELLOW}[SKIP] 32-bit test skipped (gcc-multilib not installed).${RESET}"
fi

make -s fclean

# Summary
echo -e "\n${CYAN}Summary :${RESET}"
echo -e "Total Pass : ${GREEN}${PASSED}${RESET}"
echo -e "Total Diff : ${YELLOW}${DIFF}${RESET}"
echo -e "Total Fail : ${RED}${FAILED}${RESET}"

if [ "$FAILED" -eq 0 ]; then
	echo -e "${GREEN}All tests passed successfully!${RESET}"
	exit 0
else
	echo -e "${RED}Some tests failed.${RESET}"
	exit 1
fi
