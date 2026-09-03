#!/usr/bin/env bash

# Colors
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
RESET="\033[0m"

FT_STRACE="./ft_strace"
BUILD_DIR="./bin"
PASSED=0
TOTAL=0

mkdir -p "$BUILD_DIR"

echo -e "${CYAN}=== Compiling Test Helpers ===${RESET}"

# Compile 64-bit test binaries
gcc -Wall -Wextra test/basic_test.c -o "$BUILD_DIR/basic_64"
gcc -Wall -Wextra test/signal_test.c -o "$BUILD_DIR/signal_64"
gcc -Wall -Wextra test/errno_test.c -o "$BUILD_DIR/errno_64"

# Attempt 32-bit compilation
SUPPORTS_32=1
if gcc -m32 -Wall -Wextra test/basic_test.c -o "$BUILD_DIR/basic_32" 2>/dev/null; then
	echo -e "${GREEN}[OK] 32-bit compiler available.${RESET}"
	gcc -m32 -Wall -Wextra test/signal_test.c -o "$BUILD_DIR/signal_32"
	gcc -m32 -Wall -Wextra test/errno_test.c -o "$BUILD_DIR/errno_32"
else
	echo -e "${YELLOW}[WARN] 32-bit compiler not available. Skipping 32-bit tests.${RESET}"
	SUPPORTS_32=0
fi

run_test()
{
	local test_name="$1"
	local cmd="$2"
	shift 2
	local patterns=("$@")

	TOTAL=$((TOTAL + 1))
	echo -e "\n${CYAN}[TEST $TOTAL] $test_name${RESET}"
	echo "Running: $cmd"

	output=$(eval "$cmd" 2>&1)
	
	local all_matched=1
	for pattern in "${patterns[@]}"; do
		if ! echo "$output" | grep -Eq -e "$pattern" --; then
			all_matched=0
			echo -e "${RED}[FAIL] Missing pattern: '$pattern'${RESET}"
		fi
	done

	if [ $all_matched -eq 1 ]; then
		echo -e "${GREEN}[PASS] All patterns matched!${RESET}"
		PASSED=$((PASSED + 1))
	else
		echo -e "${YELLOW}--- Output ---${RESET}\n$output\n${YELLOW}--------------${RESET}"
	fi
}

echo -e "\n${CYAN}=== Starting FT_STRACE Tests ===${RESET}"

# Arguments and Binary Check
run_test "No arguments" "$FT_STRACE" "must have PROG"
run_test "Non-existent binary" "$FT_STRACE /non/existent/path" "execvpe|No such file"

# Basic 64-bit execution & Exit Codes
run_test "Exit status code 42" "$FT_STRACE $BUILD_DIR/basic_64" "exited with 42"

# Signal Handling
run_test "SIGSEGV Crash Detection" "$FT_STRACE $BUILD_DIR/signal_64 segv" "--- SIGSEGV" "killed by SIGSEGV"
run_test "SIGFPE Crash Detection" "$FT_STRACE $BUILD_DIR/signal_64 fpe" "--- SIGFPE" "killed by SIGFPE"
run_test "SIGUSR1 Caught Signal" "$FT_STRACE $BUILD_DIR/signal_64 usr1" "--- SIGUSR1" "exited with 0"
run_test "SIGTERM Termination" "$FT_STRACE $BUILD_DIR/signal_64 term" "killed by SIGTERM"

# 32-bit Compatibility
if [ $SUPPORTS_32 -eq 1 ]; then
	run_test "No arguments" "$FT_STRACE" "must have PROG"
	run_test "Non-existent binary" "$FT_STRACE /non/existent/path" "execvpe|No such file"
	run_test "Exit status code 42" "$FT_STRACE $BUILD_DIR/basic_32" "exited with 42"
	run_test "SIGSEGV Crash Detection" "$FT_STRACE $BUILD_DIR/signal_32 segv" "--- SIGSEGV" "killed by SIGSEGV"
	run_test "SIGFPE Crash Detection" "$FT_STRACE $BUILD_DIR/signal_32 fpe" "--- SIGFPE" "killed by SIGFPE"
	run_test "SIGUSR1 Caught Signal" "$FT_STRACE $BUILD_DIR/signal_32 usr1" "--- SIGUSR1" "exited with 0"
	run_test "SIGTERM Termination" "$FT_STRACE $BUILD_DIR/signal_32 term" "killed by SIGTERM"
fi

# Summary
echo -e "\n${CYAN}================ SUMMARY ================${RESET}"
if [ $PASSED -eq $TOTAL ]; then
	echo -e "${GREEN}ALL TESTS PASSED: $PASSED / $TOTAL${RESET}"
else
	echo -e "${RED}SOME TESTS FAILED: $PASSED / $TOTAL passed${RESET}"
fi

# Cleanup
rm -rf "$BUILD_DIR"
