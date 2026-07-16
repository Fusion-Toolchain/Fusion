CC = gcc
CFLAGS = -g -Wall -Wextra -DFUSION_DEBUG
LDFLAGS = -lm

INCLUDE_DIR := ../include
LIB_PATH := ../libfusion.so
SRC_DIR := src

TEST_SRCS := $(wildcard $(SRC_DIR)/*.c)
TARGETS := $(addprefix test_,$(basename $(notdir $(TEST_SRCS))))

all: compile test

compile: $(TARGETS)

test_%: $(SRC_DIR)/%.c
	$(CC) -I$(INCLUDE_DIR) $(CFLAGS) -o $@ $^ $(LIB_PATH) $(LDFLAGS)

test: $(TARGETS)
	@for target in $(TARGETS); do \
		echo "=== Running $$target ==="; \
		valgrind --leak-check=full ./$$target || exit 1; \
	done

clean:
	@if [ -n "$(TARGETS)" ]; then rm -f $(TARGETS); fi
	@echo "Clear"

.PHONY: all compile test clean