INCLUDE_DIR := include
SRC_DIR     := src
EXAMPLE_DIR := example
BUILD_DIR   := .build
CC          := gcc
CFLAGS      := -g -I$(INCLUDE_DIR) -O3 -MMD -MP -fPIC
CFLAGS      += -Wextra -Wall -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE

ifeq ($(DEBUG),Y)
CFLAGS += -DFUSION_DEBUG
endif

SO_LDFLAGS := -shared -Wl,-T,$(SRC_DIR)/linker.ld
FUSION_SO  := libfusion.so
OUT_FILE   := main

# ========================
# CORE
# ========================
CORE_SRC := \
	$(SRC_DIR)/Core/Error_Tree/fus_tracetree.c \
	$(SRC_DIR)/Core/instance_controller.c \
	$(SRC_DIR)/Core/Compiler/compiler_pipeline.c \
	$(SRC_DIR)/Core/BufferSystem/buffer_mounter.c \
	$(SRC_DIR)/Core/Backend_System/backend_loader.c \
	$(SRC_DIR)/Core/Backend_System/backend_inject.c \
	$(SRC_DIR)/Core/IO_Sytem/io_interface.c \
	$(SRC_DIR)/Core/IO_Sytem/io_file.c \
	$(SRC_DIR)/Core/Fdb_System/fdb_filemount.c \
	$(SRC_DIR)/Core/Linker_System/linker_hashtable.c \
	$(SRC_DIR)/Core/Linker_System/linker_interface.c \
	$(SRC_DIR)/Core/Linker_System/linker_pipeline.c \
	$(SRC_DIR)/Core/Memory/fus_arena.c \
	$(SRC_DIR)/Core/Memory/fus_slab.c \
	$(SRC_DIR)/Core/Memory/fus_handle.c \
	$(SRC_DIR)/Core/Memory/fus_larger_block.c \
	$(SRC_DIR)/Core/IRTypes/fus_hidrmount.c

CORE_OBJ := $(CORE_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
CORE_LIB := $(BUILD_DIR)/libfusion_core.a

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
	$(SRC_DIR)/Backend/X86/InstructionSets/x86_lea.c \
	$(SRC_DIR)/Backend/X86/InstructionSets/x86_stack.c \
	$(SRC_DIR)/Backend/X86/InstructionSets/x86_syscall.c

BACKEND_OBJ := $(BACKEND_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
BACKEND_LIB := $(BUILD_DIR)/libfusion_x86.a

# ========================
# BUILD FINAL
# ========================
.PHONY: example

all: lib

lib: $(FUSION_SO)

example:
	$(MAKE) -C example
test:
	$(MAKE) -C test compile

$(CORE_LIB): $(CORE_OBJ)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

$(BACKEND_LIB): $(BACKEND_OBJ)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

$(FUSION_SO): $(CORE_LIB) $(BACKEND_LIB)
	$(CC) $(SO_LDFLAGS) -o $@ \
		-Wl,--whole-archive $(CORE_LIB) $(BACKEND_LIB) -Wl,--no-whole-archive

# ========================
# COMPILAÇÃO
# ========================
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/example/%.o: $(EXAMPLE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ========================
# CLEAN
# ========================
clean:
	$(MAKE) -C test clean
	$(MAKE) -C example clean
	rm -rf $(BUILD_DIR) $(FUSION_SO) main *.bin

# ========================
# DEPENDÊNCIAS
# ========================
DEP := $(CORE_OBJ:.o=.d) $(BACKEND_OBJ:.o=.d)
-include $(DEP)