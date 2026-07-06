//===-- RV32IRegisterInfo.h - RV32I Register Information Impl ---*- C++ -*-===//
//
// This file implements the TargetRegisterInfo interface. It is responsible for 
// answering queries about the CPU's register file: which registers are reserved? 
// which are callee-saved? and how do we lower stack frame indices into 
// physical register offsets?
//
//===----------------------------------------------------------------------===//

#pragma once

#include "RV32I.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_ENUM
#include "RV32IGenRegisterInfo.inc"

#define GET_REGINFO_HEADER
#include "RV32IGenRegisterInfo.inc"

namespace llvm {

class RV32ISubtarget;
class TargetInstrInfo;
class MachineFunction;

/// RV32IRegisterInfo:
/// Inherits from the TableGen auto-generated `RV32IGenRegisterInfo`.
class RV32IRegisterInfo : public RV32IGenRegisterInfo {
  const RV32ISubtarget &Subtarget;

public:
  RV32IRegisterInfo(RV32ISubtarget const &ST);

  /// getCalleeSavedRegs: Returns the list of CSRs (x8, x9, x18-x27, etc.).
  MCPhysReg const *getCalleeSavedRegs(MachineFunction const *MF) const override;

  /// getCallPreservedMask: A bitmask of preserved registers for the call graph.
  uint32_t const *getCallPreservedMask(MachineFunction const &MF,
                                       CallingConv::ID) const override;

  /// getReservedRegs: Returns a bit vector of registers the compiler is NOT 
  /// allowed to use (like x0/zero, x2/sp).
  BitVector getReservedRegs(MachineFunction const &MF) const override;

  /// eliminateFrameIndex:
  /// Very important function! Converts an abstract `FrameIndex` (like FI#2) 
  /// into a physical memory access relative to the Stack Pointer (e.g., `sp + 12`).
  bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  /// getFrameRegister: Returns the register used for stack addressing (usually x2/sp).
  Register getFrameRegister(MachineFunction const &MF) const override;
};

} 