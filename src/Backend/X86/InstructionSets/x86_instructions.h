#ifndef X86_INTERNAL_FUNCTIONS_SETS_H
#define X86_INTERNAL_FUNCTIONS_SETS_H
#include "../x86_types.h"
#include "../x86_familys.h"

// MOV SET
bool X86_CaseMountMovImmReg(X86BackendContext* backend_ctx);
bool X86_CaseMountMovRegReg(X86BackendContext* backend_ctx);
bool X86_CaseMountMovMemImm(X86BackendContext* backend_ctx);
bool X86_CaseMountMovSymReg(X86BackendContext* backend_ctx);

// ADD SET
bool X86_CaseMountAddRegReg(X86BackendContext* backend_ctx);
bool X86_CaseMountAddImmReg(X86BackendContext* backend_ctx);

// CALL SET
bool X86_CaseMountCallReg(X86BackendContext* backend_ctx);
// TODO: X86_CaseMountCallImm, nao existe este conseito, mudar para REL32 com OPERAND TIPO SYMBOL com placeholder
bool X86_CaseMountCallImm(X86BackendContext* backend_ctx);

// RET SET
bool X86_MountRet(X86BackendContext* backend_ctx);
bool X86_CaseMountLeaRegMem(X86BackendContext* backend_ctx);

#endif