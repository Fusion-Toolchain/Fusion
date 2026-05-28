CC = gcc
CFLAGS = -g -Wall -Wextra -Iinclude -DFUSION_DEBUG
LDFLAGS = -lm

INCLUDE_DIR := ../include
TARGET = test_slab
TEST_SRC = slab_test.c

all: compile test

compile: $(TEST_SRC)
	$(CC) -I$(INCLUDE_DIR) $(CFLAGS) -o $(TARGET) $^ ../libfusion.so $(LDFLAGS)

test:
	valgrind --leak-check=full ./$(TARGET)

.PHONY: all compile test clean
clean:
	rm -f $(TARGET)