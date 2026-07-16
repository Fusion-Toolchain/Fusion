#ifndef X86_INTERNAL_FUNCTIONS_SETS_H
#define X86_INTERNAL_FUNCTIONS_SETS_H
#include "../x86_types.h"
#include "../x86_familys.h"

// MOV SET
bool X86_CaseMountMovImmReg(X86BackendContext* backend_ctx);
bool X86_CaseMountMovRegReg(X86BackendContext* backend_ctx);
bool X86_CaseMountMovMemImm(X86BackendContext* backend_ctx);
bool X86_CaseMountMovSymReg(X86BackendContext* backend_ctx);
bool X86_CaseMountMovRegMem(X86BackendContext* backend_ctx);

// ADD SET
bool X86_CaseMountAddRegReg(X86BackendContext* backend_ctx);
bool X86_CaseMountAddImmReg(X86BackendContext* backend_ctx);

// CALL SET
bool X86_CaseMountCallReg(X86BackendContext* backend_ctx);
bool X86_CaseMountCallRel32(X86BackendContext* backend_ctx);

// RET SET
bool X86_MountRet(X86BackendContext* backend_ctx);
bool X86_CaseMountLeaRegMem(X86BackendContext* backend_ctx);

//PUSH
bool X86_CaseMountPushReg(X86BackendContext* backend_ctx);
bool X86_CaseMountPushImm(X86BackendContext* backend_ctx);

// POP
bool X86_CaseMountPopReg(X86BackendContext* backend_ctx);
bool X86_CaseMountPopImm(X86BackendContext* backend_ctx);

// SYSCALL
bool X86_CaseMountSyscall(X86BackendContext* backend_ctx);

#endif