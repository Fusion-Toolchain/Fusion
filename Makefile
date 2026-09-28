INCLUDE_DIR := include
SRC_DIR     := src
EXAMPLE_DIR := example
BUILD_DIR   := .build
CC          := gcc
CFLAGS      := -std=gnu23 -I$(INCLUDE_DIR) -O3 -MMD -MP -fPIC
CFLAGS      += -Wextra -Wall -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE

ifeq ($(DEBUG),Y)
CFLAGS += -DFUSION_DEBUG
endif

SO_LDFLAGS := -shared -Wl,-T,$(SRC_DIR)/linker.ld
FUSION_SO  := libfusion.so

# ========================
# CORE
# ========================
CORE_SRC := \
	$(SRC_DIR)/Error_Tree/fus_tracetree.c \
	$(SRC_DIR)/instance_controller.c \
	$(SRC_DIR)/Compiler/compiler_pipeline.c \
	$(SRC_DIR)/BufferSystem/buffer_mounter.c \
	$(SRC_DIR)/BufferSystem/bufferio_utils.c \
	$(SRC_DIR)/Backend_System/backend_loader.c \
	$(SRC_DIR)/Backend_System/backend_inject.c \
	$(SRC_DIR)/Backend_System/backend_dynamic.c \
	$(SRC_DIR)/IO_Sytem/io_interface.c \
	$(SRC_DIR)/IO_Sytem/io_file.c \
	$(SRC_DIR)/Linker_System/linker_hashtable.c \
	$(SRC_DIR)/Linker_System/linker_interface.c \
	$(SRC_DIR)/Linker_System/linker_pipeline.c \
	$(SRC_DIR)/Memory/fus_arena.c \
	$(SRC_DIR)/Memory/fus_slab.c \
	$(SRC_DIR)/Memory/fus_handle.c \
	$(SRC_DIR)/Memory/fus_larger_block.c \
	$(SRC_DIR)/IRTypes/fus_hidrmount.c \
	$(SRC_DIR)/IRTypes/fus_hidregistrer.c \

CORE_OBJ := $(CORE_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
CORE_LIB := libfusion.a

# ========================
# BUILD FINAL
# ========================
.PHONY: example

all: lib

lib: $(FUSION_SO)

example:
	$(MAKE) -C example

$(CORE_LIB): $(CORE_OBJ)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

$(FUSION_SO): $(CORE_LIB)
	$(CC) $(SO_LDFLAGS) -o $@ -Wl,--whole-archive $(CORE_LIB) $(STATICS_BACKENDS) -Wl,--no-whole-archive

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
	@$(MAKE) -C test clean
	@$(MAKE) -C example clean
	rm -rf $(BUILD_DIR) $(FUSION_SO) $(CORE_LIB) main *.bin

# ========================
# DEPENDÊNCIAS
# ========================
DEP := $(CORE_OBJ:.o=.d) $(BACKEND_OBJ:.o=.d)
-include $(DEP)