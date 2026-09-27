#ifndef X86_INTERNAL_FUNCTIONS_SETS_H
#define X86_INTERNAL_FUNCTIONS_SETS_H

#include "../x86_types.h"
#include "../x86_familys.h"

/* MOV */
bool X86_CaseMountMovImmReg(X86BackendContext *ctx);
bool X86_CaseMountMovRegReg(X86BackendContext *ctx);
bool X86_CaseMountMovMemImm(X86BackendContext *ctx);
bool X86_CaseMountMovSymReg(X86BackendContext *ctx);
bool X86_CaseMountMovRegMem(X86BackendContext *ctx);

/* ALU */
bool X86_CaseMountAddRegReg(X86BackendContext *ctx);
bool X86_CaseMountAddImmReg(X86BackendContext *ctx);
bool X86_CaseMountCmpRegReg(X86BackendContext *ctx);
bool X86_CaseMountCmpRegImm(X86BackendContext *ctx);

/* Control / Stack */
bool X86_CaseMountCallReg(X86BackendContext *ctx);
bool X86_CaseMountCallRel32(X86BackendContext *ctx);
bool X86_MountRet(X86BackendContext *ctx);
bool X86_CaseMountLeaRegMem(X86BackendContext *ctx);
bool X86_CaseMountPushReg(X86BackendContext *ctx);
bool X86_CaseMountPushImm(X86BackendContext *ctx);
bool X86_CaseMountPopReg(X86BackendContext *ctx);
bool X86_CaseMountPopImm(X86BackendContext *ctx);
bool X86_CaseMountSyscall(X86BackendContext *ctx);

#endif
