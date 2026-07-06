//===-- RV32IMCTargetDesc.cpp - RV32I Target Descriptions ---------*- C++ -*-===//
//
// This file initializes all the core MC-layer components (AsmInfo, InstrInfo,
// RegisterInfo, SubtargetInfo) and registers them with LLVM's `TargetRegistry`.
// 
// Without this file, tools like `llvm-mc` or `llvm-objdump` wouldn't know 
// anything about the RV32I architecture!
//
//===----------------------------------------------------------------------===//

#include "RV32IMCTargetDesc.h"
#include "RV32IInstPrinter.h"
#include "../RV32ITargetInfo.h"
#include "RV32IMCAsmInfo.h"
#include "RV32ITargetStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "RV32IGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "RV32IGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "RV32IGenRegisterInfo.inc"

static MCInstrInfo *createRV32IMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitRV32IMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createRV32IMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitRV32IMCRegisterInfo(X, RV32I::X1); // X1 (ra) is the return address register
  return X;
}

static MCSubtargetInfo *
createRV32IMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createRV32IMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCAsmInfo *createRV32IMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT,
                                       const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new RV32IMCAsmInfo(TT, Options);
  return MAI;
}

static MCInstPrinter *createRV32IMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new RV32IInstPrinter(MAI, MII, MRI);
}

MCTargetStreamer *
llvm::createRV32ITargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS,
                                   MCInstPrinter *InstPrint) {
  return new RV32ITargetAsmStreamer(S, OS);
}

/// LLVMInitializeRV32ITargetMC:
/// The global initialization hook!
/// This function is called dynamically (often via plugins) to hook our custom 
/// implementations into the LLVM infrastructure.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRV32ITargetMC() {
  for (Target *T : {&getTheRV32ITarget()}) {
    RegisterMCAsmInfoFn X(*T, createRV32IMCAsmInfo);
    TargetRegistry::RegisterMCInstrInfo(*T, createRV32IMCInstrInfo);
    TargetRegistry::RegisterMCRegInfo(*T, createRV32IMCRegisterInfo);
    TargetRegistry::RegisterMCSubtargetInfo(*T, createRV32IMCSubtargetInfo);
    TargetRegistry::RegisterMCInstPrinter(*T, createRV32IMCInstPrinter);
    TargetRegistry::RegisterMCCodeEmitter(*T, createRV32IMCCodeEmitter);
    TargetRegistry::RegisterAsmTargetStreamer(*T, createRV32ITargetAsmStreamer);
  }
}
