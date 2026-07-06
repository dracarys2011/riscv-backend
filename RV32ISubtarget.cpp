//===-- RV32ISubtarget.cpp - RV32I Subtarget Information ------*- C++ -*-===//
//
// The Subtarget implementation file. Here we initialize all the backend 
// components that require knowledge of the target's specific features.
//
//===----------------------------------------------------------------------===//

#include "RV32ISubtarget.h"
#include "RV32I.h"
#include "llvm/TargetParser/TargetParser.h"

using namespace llvm;

#define DEBUG_TYPE "rv32i-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "RV32IGenSubtargetInfo.inc"

/// Anchor function to pin the vtable to this file.
void RV32ISubtarget::anchor() {}

/// Constructor:
/// We invoke the auto-generated constructor `RV32IGenSubtargetInfo`, then 
/// initialize our custom lowering and instruction components, passing `*this` 
/// (the Subtarget) so they can query features (e.g., "is the M extension enabled?").
/// Finally, `ParseSubtargetFeatures` parses the command line flags.
RV32ISubtarget::RV32ISubtarget(const Triple &TT, const StringRef CPU,
                               const StringRef FS, const TargetMachine &TM)
    : RV32IGenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS),
      InstrInfo(*this),
      FrameLowering(*this),
      TLInfo(TM, *this) {
  ParseSubtargetFeatures(CPU, CPU, FS);
}
