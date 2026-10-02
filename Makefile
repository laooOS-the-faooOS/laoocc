CC       = gcc
CFLAGS   = -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS = -I./api

TARGET    = laoocc
API_CHECK = laoocc-api-check

SRC = $(shell find src -type f -name '*.c')
OBJ = $(SRC:.c=.o)

MAIN_OBJ  = bin/main.o
CHECK_OBJ = bin/api_check.o

.PHONY: all clean check

all: check $(TARGET)

check: $(API_CHECK)
	@echo
	@echo "==> Running API check"
	@./$(API_CHECK)

$(TARGET): $(OBJ) $(MAIN_OBJ)
	@echo "==> API check passed, building laoocc"
	$(CC) $(CFLAGS) $^ -o $@

$(API_CHECK): $(OBJ) $(CHECK_OBJ)
	@echo "==> Linking API checker"
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	find src bin tests -type f -name '*.o' -delete
	rm -f $(TARGET) $(API_CHECK)
