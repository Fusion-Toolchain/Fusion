CC := gcc

CFLAG := -I../include
FUS_LIB := -L../ -lfusion -Wl,-rpath,'$$ORIGIN'

SRC_DIR := src
EXAMPLE_SRCS := $(wildcard $(SRC_DIR)/*.c)
TARGETS := $(addprefix example_,$(basename $(notdir $(EXAMPLE_SRCS))))

all: $(TARGETS)

example_%: $(SRC_DIR)/%.c
	$(CC) $(CFLAG) $^ $(FUS_LIB) -o $@

clean:
	@if [ -n "$(TARGETS)" ]; then rm -f $(TARGETS); fi
	@echo "Clear"

.PHONY: all clean
