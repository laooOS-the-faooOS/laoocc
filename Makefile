CC       = gcc
CXX      = g++
AR       = ar
CFLAGS   = -std=c11 -Wall -Wextra -Wpedantic -O2
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2
CPPFLAGS = -I./api -I./frontend/lexer/api
ARFLAGS  = rcs

SRC = $(shell find . -type f -name '*.c')
OBJ = $(SRC:.c=.o)

.PHONY: all objects clean

all: objects

objects: $(OBJ)

%.o: %.c
	@mkdir -p $(dir $@)
	@printf "  CC    %-45s %s\n" "$<" "$(CFLAGS)"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c "$<" -o "$@"

clean:
	@find . -type f -name '*.o' -delete
	@rm -f laoocc
