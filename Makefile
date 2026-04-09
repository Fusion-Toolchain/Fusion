INCLUDE_DIR := include
SRC_DIR := src
CFLAG := -I$(INCLUDE_DIR) -O1
LINKER_LD := -Wl,-T,$(SRC_DIR)/linker.ld
OUT_FILE := main

OBJ := $(SRC_DIR)/main.c \
  $(SRC_DIR)/Backend/X86/x86_pipeline.c \
  $(SRC_DIR)/Backend/X86/x86_interface.c \
  $(SRC_DIR)/Core/fusion_core.c \
  $(SRC_DIR)/Core/fus_file.c \
  $(SRC_DIR)/Memory/fus_arena.c \
  $(SRC_DIR)/Memory/fus_handle.c

all:
	gcc $(LINKER_LD) $(CFLAG) $(OBJ) -o $(OUT_FILE)

clean:
	rm -rf $(OUT_FILE)