//===-- RV32IMCTargetDesc.h - RV32I Target Descriptions ---------*- C++ -*-===//
//
// The MCTargetDesc (Machine Code Target Description) layer is the absolute 
// lowest level of the LLVM backend. 
// It is completely independent of LLVM IR, SelectionDAG, or MachineInstrs.
// Its sole purpose is to handle `MCInst`s: printing them as text (InstPrinter) 
// or encoding them as binary bits (MCCodeEmitter).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RV32I_MCTARGETDESC_RV32IMCTARGETDESC_H
#define LLVM_LIB_TARGET_RV32I_MCTARGETDESC_RV32IMCTARGETDESC_H

#include "llvm/Config/config.h"
#include "llvm/Support/DataTypes.h"
#include <memory>

namespace llvm {
class Target;
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class MCTargetStreamer;
class MCStreamer;
class MCInstPrinter;
class formatted_raw_ostream;
class StringRef;
class Triple;

/// getTheRV32ITarget: The global target instance used for registration.
Target &getTheRV32ITarget();

/// createRV32IMCCodeEmitter:
/// Creates the component that converts an `MCInst` into actual binary machine code.
MCCodeEmitter *createRV32IMCCodeEmitter(const MCInstrInfo &MCII,
                                        MCContext &Ctx);

/// createRV32ITargetAsmStreamer:
/// Creates the component that handles printing custom directives or assembly 
/// formats specific to RISC-V.
MCTargetStreamer *createRV32ITargetAsmStreamer(MCStreamer &S,
                                               formatted_raw_ostream &OS,
                                               MCInstPrinter *InstPrint);
} // end namespace llvm

// Defines symbolic names for RV32I registers.
#define GET_REGINFO_ENUM
#include "RV32IGenRegisterInfo.inc"

// Defines symbolic names for the RV32I instructions.
#define GET_INSTRINFO_ENUM
#include "RV32IGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "RV32IGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_RV32I_MCTARGETDESC_RV32IMCTARGETDESC_H
