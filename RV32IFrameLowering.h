//===-- RV32IFrameLowering.h - Frame info for RV32I Target ------*- C++ -*-===//
//
// The FrameLowering class is responsible for managing the function's stack 
// frame. When a function executes, it often needs temporary memory (the stack) 
// to save return addresses, callee-saved registers, or local variables.
//
// This class inserts the Prologue (allocating the stack) at the start of a 
// function, and the Epilogue (deallocating the stack) at the end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RV32I_RV32IFRAMELOWERING_H
#define LLVM_LIB_TARGET_RV32I_RV32IFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class RV32ISubtarget;

/// RV32IFrameLowering:
/// Inherits from TargetFrameLowering to implement RV32I ABI rules.
class RV32IFrameLowering : public TargetFrameLowering {
protected:
  const RV32ISubtarget &STI;

public:
  /// Constructor:
  /// We tell LLVM that the RISC-V stack grows *downward* (from high memory to low), 
  /// and that the stack pointer must be aligned to 16 bytes (ILP32 ABI).
  explicit RV32IFrameLowering(const RV32ISubtarget &STI)
      : TargetFrameLowering(StackGrowsDown,
                            /*StackAlignment=*/Align(16),
                            /*LocalAreaOffset=*/0),
        STI(STI) {}

  /// emitPrologue:
  /// Inserts instructions at the start of a function to allocate stack space 
  /// (e.g., `addi sp, sp, -32`).
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  /// emitEpilogue:
  /// Inserts instructions at the end of a function to deallocate stack space 
  /// (e.g., `addi sp, sp, 32`) right before the `ret` instruction.
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  /// hasFPImpl:
  /// Determines if the current function needs a dedicated Frame Pointer (FP/x8).
  /// (Note: In LLVM 23, `hasFP` was made non-virtual, so we override `hasFPImpl`).
  bool hasFPImpl(const MachineFunction &MF) const override;

  /// eliminateCallFramePseudoInstr:
  /// During lowering, we inserted pseudo-instructions like `ADJCALLSTACKDOWN` 
  /// and `ADJCALLSTACKUP`. This function replaces those pseudos with actual 
  /// stack adjustments, or deletes them entirely if they aren't needed.
  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;
};
} // end namespace llvm

#endif // LLVM_LIB_TARGET_RV32I_RV32IFRAMELOWERING_H
