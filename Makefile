INCLUDE_DIR := include
SRC_DIR := src
BUILD_DIR := .build

CC := gcc
AR := ar
CFLAGS := -DFUSION_DEBUG -g -I$(INCLUDE_DIR) -O2 -MMD -MP
LDFLAGS := -Wl,-T,$(SRC_DIR)/linker.ld

OUT_FILE := main

# ========================
# CORE
# ========================
CORE_SRC := \
  $(SRC_DIR)/Core/instance_controller.c \
  $(SRC_DIR)/Core/Compiler/compiler_pipeline.c \
  $(SRC_DIR)/Core/BufferSystem/buffer_mounter.c \
  $(SRC_DIR)/Core/Backend_System/backend_loader.c \
  $(SRC_DIR)/Core/IO_Sytem/io_interface.c \
  $(SRC_DIR)/Core/IO_Sytem/io_file.c \
  $(SRC_DIR)/Core/Fdb_System/fdb_filemount.c \
  $(SRC_DIR)/Core/Linker_System/linker_hashtable.c \
  $(SRC_DIR)/Core/Linker_System/linker_interface.c \
  $(SRC_DIR)/Core/Linker_System/linker_pipeline.c \
  $(SRC_DIR)/Core/Memory/fus_arena.c \
  $(SRC_DIR)/Core/Memory/fus_slab.c \
  $(SRC_DIR)/Core/Memory/fus_handle.c \
  $(SRC_DIR)/Core/Memory/fus_larger_block.c

CORE_OBJ := $(CORE_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
CORE_LIB := $(BUILD_DIR)/libcore.a

# ========================
# BACKEND
# ========================
BACKEND_SRC := \
  $(SRC_DIR)/Backend/X86/x86_pipeline.c \
  $(SRC_DIR)/Backend/X86/x86_interface.c \
  $(SRC_DIR)/Backend/X86/x86_helpers.c \
  $(SRC_DIR)/Backend/X86/InstructionSets/x86_mov.c \
  $(SRC_DIR)/Backend/X86/InstructionSets/x86_add.c \
  $(SRC_DIR)/Backend/X86/InstructionSets/x86_call.c \
  $(SRC_DIR)/Backend/X86/InstructionSets/x86_ret.c \
  $(SRC_DIR)/Backend/X86/InstructionSets/x86_lea.c

BACKEND_OBJ := $(BACKEND_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
BACKEND_LIB := $(BUILD_DIR)/libbackend.a

# ========================
# MAIN
# ========================
MAIN_SRC := $(SRC_DIR)/main.c
MAIN_OBJ := $(BUILD_DIR)/main.o

TEST_SRC := \
 $(SRC_DIR)/Test/slab_test.c

# ========================
# BUILD FINAL
# ========================
all: $(OUT_FILE)

$(OUT_FILE): $(MAIN_OBJ) $(BACKEND_LIB) $(CORE_LIB)
	$(CC) $(LDFLAGS) $(MAIN_OBJ) \
	-Wl,--whole-archive $(BACKEND_LIB) $(CORE_LIB) -Wl,--no-whole-archive \
	-o $@

# ========================
# LIBS
# ========================
$(CORE_LIB): $(CORE_OBJ)
	$(AR) rcs $@ $^

$(BACKEND_LIB): $(BACKEND_OBJ)
	$(AR) rcs $@ $^

# ========================
# COMPILAÇÃO
# ========================
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ========================
# CLEAN
# ========================
clean:
	rm -rf $(BUILD_DIR) $(OUT_FILE)

# ========================
# DEPENDÊNCIAS
# ========================
DEP := $(CORE_OBJ:.o=.d) $(BACKEND_OBJ:.o=.d) $(MAIN_OBJ:.o=.d)
-include $(DEP)