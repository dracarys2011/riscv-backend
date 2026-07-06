#ifndef LLVM_LIB_TARGET_RV32I_RV32I_H
#define LLVM_LIB_TARGET_RV32I_RV32I_H

#include "llvm/Pass.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class RV32ITargetMachine;
class FunctionPass;
class PassRegistry;

// Add function declarations for any target-specific passes here
// void initializeRV32IPasses(PassRegistry &);
FunctionPass *createRV32IISelDag(RV32ITargetMachine &TM, CodeGenOptLevel OptLevel);

} // end namespace llvm

#endif // LLVM_LIB_TARGET_RV32I_RV32I_H
