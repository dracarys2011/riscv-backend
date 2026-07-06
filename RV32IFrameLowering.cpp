//===-- RV32IFrameLowering.cpp - Frame info for RV32I Target ----*- C++ -*-===//
//
// This file contains the RV32I implementation of TargetFrameLowering class.
// It implements the logic for creating and destroying stack frames.
//
//===----------------------------------------------------------------------===//

#include "RV32IFrameLowering.h"
#include "RV32I.h"
#include "RV32IInstrInfo.h"
#include "RV32ISubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

/// hasFPImpl:
/// Determines if the function requires a dedicated Frame Pointer (FP) in `x8`.
/// We need an FP if frame pointer elimination is disabled, or if the function 
/// has variable-sized objects (like `alloca`s) where the stack size isn't 
/// known at compile time.
bool RV32IFrameLowering::hasFPImpl(const MachineFunction &MF) const {
  // Choice: For this simple educational backend, we do not implement the logic
  // to save/restore the frame pointer in emitPrologue/emitEpilogue. Therefore,
  // we must tell the register allocator that we do NOT have a frame pointer,
  // forcing it to always use the Stack Pointer (SP) for FrameIndex elimination.
  return false;
}

/// emitPrologue:
/// Runs at the very beginning of the function.
/// 1. Calculates the total size of the stack frame needed for the function.
/// 2. Generates an `addi sp, sp, -StackSize` instruction.
/// 3. (Optional, if using FP): Sets up the Frame Pointer.
void RV32IFrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  // Choice: Simple RV32I ILP32 prologue. For frames > 2047 bytes,
  // a LUI+ADD sequence would be needed; we limit to ADDI-range for now.
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const RV32IInstrInfo *TII =
      static_cast<const RV32IInstrInfo *>(MF.getSubtarget().getInstrInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL;

  uint64_t StackSize = MFI.getStackSize();
  if (StackSize == 0 && !MFI.adjustsStack())
    return;

  // SP = SP - StackSize
  BuildMI(MBB, MBBI, DL, TII->get(RV32I::ADDI), RV32I::X2)
      .addReg(RV32I::X2)
      .addImm(-static_cast<int64_t>(StackSize))
      .setMIFlag(MachineInstr::FrameSetup);
}

/// emitEpilogue:
/// Runs at the very end of the function, immediately before the `ret`.
/// 1. Restores the stack pointer by generating an `addi sp, sp, +StackSize`.
void RV32IFrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  // Choice: Simple RV32I ILP32 epilogue.
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const RV32IInstrInfo *TII =
      static_cast<const RV32IInstrInfo *>(MF.getSubtarget().getInstrInfo());
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc DL;
  if (MBBI != MBB.end()) DL = MBBI->getDebugLoc();

  uint64_t StackSize = MFI.getStackSize();
  if (StackSize == 0)
    return;

  // SP = SP + StackSize
  BuildMI(MBB, MBBI, DL, TII->get(RV32I::ADDI), RV32I::X2)
      .addReg(RV32I::X2)
      .addImm(static_cast<int64_t>(StackSize))
      .setMIFlag(MachineInstr::FrameDestroy);
}

/// eliminateCallFramePseudoInstr:
/// Whenever we make a function call, the backend creates `ADJCALLSTACKDOWN` 
/// and `ADJCALLSTACKUP` pseudo-instructions to allocate stack space for the 
/// outgoing arguments. 
/// In this function, we replace those pseudos with actual `addi sp, sp, ...` 
/// instructions, or if the stack space was pre-allocated in the prologue, we 
/// simply erase the pseudos!
MachineBasicBlock::iterator RV32IFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  // Choice: Only emit SP adjustments if the frame is not reserved.
  // When the call frame is reserved (i.e., not variable-sized), the
  // prologue/epilogue handles the adjustment; these pseudos are just erased.
  if (!hasReservedCallFrame(MF)) {
    const RV32IInstrInfo *TII =
        static_cast<const RV32IInstrInfo *>(MF.getSubtarget().getInstrInfo());
    DebugLoc DL = MI->getDebugLoc();
    int64_t Amount = MI->getOperand(0).getImm();

    if (Amount != 0) {
      if (MI->getOpcode() == RV32I::ADJCALLSTACKDOWN) {
        BuildMI(MBB, MI, DL, TII->get(RV32I::ADDI), RV32I::X2)
            .addReg(RV32I::X2).addImm(-Amount);
      } else if (MI->getOpcode() == RV32I::ADJCALLSTACKUP) {
        BuildMI(MBB, MI, DL, TII->get(RV32I::ADDI), RV32I::X2)
            .addReg(RV32I::X2).addImm(Amount);
      }
    }
  }

  return MBB.erase(MI);
}
