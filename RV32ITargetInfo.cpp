//===-- RV32ITargetInfo.cpp - RV32I Target Implementation -------*- C++ -*-===//
//
// This file registers our custom architecture with LLVM's Target Registry.
// Without this file, LLVM tools like `llc` wouldn't even know that the 
// "rv32i" target exists!
//
//===----------------------------------------------------------------------===//

#include "RV32ITargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

/// getTheRV32ITarget:
/// A standard singleton pattern returning the one true instance of our Target.
Target &llvm::getTheRV32ITarget() {
    static Target TheRV32ITarget;
    return TheRV32ITarget;
}

/// LLVMInitializeRV32ITargetInfo:
/// This is an initialization hook called dynamically by LLVM or by our 
/// custom compiler driver. We map the name "rv32i" to our Target struct.
/// Note that since this is an out-of-tree backend not baked into LLVM's 
/// enums, we use `Triple::UnknownArch` for the architecture enum and rely 
/// strictly on the string literal "rv32i".
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRV32ITargetInfo() {
    RegisterTarget<Triple::UnknownArch, false> X(getTheRV32ITarget(), "rv32i", "RV32I (32-bit RISC-V)", "RV32I");
}