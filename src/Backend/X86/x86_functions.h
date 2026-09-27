#ifndef X86_BACKEND_FUNCTIONS
#define X86_BACKEND_FUNCTIONS

#include "x86_types.h"
#include <stddef.h>
#include <stdbool.h>

/*
 * Encode x86Instruction_t into buffer at *offset.
 * Advances *offset on success.
 */
bool X86_MountCodeBytes(x86Instruction_t *instr, size_t *offset, uint8_t *buffer, size_t buffer_size);

#endif
