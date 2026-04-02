INCLUDE_DIR := include
SRC_DIR := src
CFLAG := -I$(INCLUDE_DIR) -O1
OUT_FILE := main

OBJ := $(SRC_DIR)/main.c \
  $(SRC_DIR)/Backend/X86/x86_pipeline.c \
  $(SRC_DIR)/Backend/X86/x86_interface.c \
  $(SRC_DIR)/fusion_core.c

all:
	gcc $(CFLAG) $(OBJ) -o $(OUT_FILE)

clean:
	rm -rf $(OUT_FILE)