#!/bin/sh

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"

RED='\033[1;31m'
GREEN='\033[1;32m'
BOLD='\033[1m'
RESET='\033[0m'

failed=0

printf '%b\n' "${BOLD}Checking build tree...${RESET}"

printf "  CHECK  Makefile"
if [ -f Makefile ]; then
    printf "  OK\n"
else
    printf "  FAIL\n"
    failed=1
fi

for dir in api src bin frontend scripts; do
    printf "  CHECK  %s/" "$dir"

    if [ -d "$dir" ]; then
        printf "  OK\n"
    else
        printf "  FAIL\n"
        failed=1
    fi
done

printf "  CHECK  C sources"
source_count=$(find . -type f -name '*.c' | wc -l)

if [ "$source_count" -gt 0 ]; then
    printf "  %s files\n" "$source_count"
else
    printf "  FAIL\n"
    failed=1
fi

printf "  CHECK  API headers"
header_count=$(find api frontend -type f -name '*.h' | wc -l)

if [ "$header_count" -gt 0 ]; then
    printf "  %s files\n" "$header_count"
else
    printf "  FAIL\n"
    failed=1
fi

printf "  CHECK  build scripts"

if [ -x scripts/check_build.sh ]; then
    printf "  OK\n"
else
    printf "  FAIL\n"
    failed=1
fi

printf "  CHECK  source tree"

if find . -type f -name '*.c' -o -type f -name '*.h' | grep -q .; then
    printf "  OK\n"
else
    printf "  FAIL\n"
    failed=1
fi

printf "\n"

if [ "$failed" -ne 0 ]; then
    printf '%b\n' "${RED}${BOLD}Build check failed.${RESET}"
    exit 1
fi

printf '%b\n' "${GREEN}${BOLD}Build good.${RESET}"
