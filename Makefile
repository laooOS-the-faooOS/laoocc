CC       = gcc
CXX      = g++
AR       = ar

CFLAGS   = -std=c11 -Wall -Wextra -Wpedantic -Ofast
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Ofast

CPPFLAGS = -I./api \
           -I./frontend/lexer/api \
           -I./frontend/preprocessor/api

ARFLAGS  = rcs

SRC = $(shell find . -type f -name '*.c' | sort)
OBJ = $(SRC:.c=.o)

.PHONY: all check-build build-source objects laoocc clean

all: check-build build-source laoocc

check-build:
	@./scripts/check_build.sh

build-source: check-build
	@printf "\033[1;32m>>> source ok.\033[0m\n"
	@printf "\033[1m>>> building source ...\033[0m\n"
	@$(MAKE) --no-print-directory objects
	@printf "\033[1;32m>>> finished source ...\033[0m\n"

objects: $(OBJ)

laoocc: bin/main.o frontend/preprocessor/src/pp_engine.o
	@printf "\033[1m>>> linking executables ...\033[0m\n"
	@printf "\033[1;34m  LD\033[0m    \033[1;37m%-45s\033[0m\n" "laoocc"
	@$(CC) $(CFLAGS) $^ -o $@
	@printf "\033[1;32m>>> linking finished.\033[0m\n"
	@printf "\033[1;35m>>> balling out :)\033[0m\n"

%.o: %.c
	@mkdir -p $(dir $@)
	@printf "\033[1;34m  CC\033[0m    \033[1;37m%-45s\033[0m \033[1;33m%s\033[0m\n" "$<" "$(CFLAGS)"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c "$<" -o "$@"

clean:
	@find . -type f -name '*.o' -print | while read -r file; do \
		printf "  CLEAN  %s\n" "$$file"; \
		rm -f "$$file"; \
	done
	@rm -f laoocc laoocc-api-check
	@printf "Cleaned.\n"
