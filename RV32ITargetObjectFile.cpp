//===-- RV32ITargetObjectFile.cpp - RV32I Object Info -----------*- C++ -*-===//
//
// Here we implement the object file lowering logic. For our basic backend, 
// the default ELF behavior provided by LLVM is sufficient.
//
//===----------------------------------------------------------------------===//

#include "RV32ITargetObjectFile.h"
#include "RV32ITargetMachine.h"

using namespace llvm;

/// Initialize:
/// We just defer to the base TargetLoweringObjectFileELF implementation to 
/// setup standard ELF sections (like .text, .data, .bss, .rodata, etc.).
void RV32IELFTargetObjectFile::Initialize(MCContext &Ctx,
                                          const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
  // Note: If RV32I had specialized sections (like `.sdata` for small data 
  // optimization), we would define and initialize them here.
}
