INCLUDE_DIR := include
SRC_DIR := src
CFLAG := -I$(INCLUDE_DIR) -O1

OBJ := $(SRC_DIR)/main.c

all:
	gcc $(CFLAG) $(OBJ) -o main