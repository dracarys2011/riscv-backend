//===-- RV32IRegisterInfo.cpp - RV32I Register Information Impl -*- C++ -*-===//
//
// This file contains the implementation of the RV32IRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "RV32IRegisterInfo.h"
#include "RV32I.h"
#include "RV32ISubtarget.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/Debug.h"

#define GET_REGINFO_TARGET_DESC
#include "RV32IGenRegisterInfo.inc"

using namespace llvm;

/// Constructor:
/// We pass `RV32I::X1` (ra) to the base class as the default return address register.
RV32IRegisterInfo::RV32IRegisterInfo(RV32ISubtarget const &ST)
    : RV32IGenRegisterInfo(RV32I::X1), Subtarget(ST) {}

MCPhysReg const *RV32IRegisterInfo::getCalleeSavedRegs(MachineFunction const *MF) const {
  // Choice: Standard RISC-V ILP32 callee-saved registers.
  // ra (X1), s0/fp (X8), s1 (X9), s2-s11 (X18-X27).
  // This list is generated from CSR_RV32I in RV32ICallingConv.td.
  return CSR_RV32I_SaveList;
}

uint32_t const *RV32IRegisterInfo::getCallPreservedMask(MachineFunction const &MF,
                                                        CallingConv::ID) const {
  // Choice: Return the bitmask corresponding to CSR_RV32I.
  // This is critical for correct LowerCall: the register mask tells the
  // register allocator which registers survive across a call.
  return CSR_RV32I_RegMask;
}

/// getReservedRegs:
/// We reserve X0 (zero register) because its value is hardwired to 0.
/// We reserve X2 (stack pointer) because we don't want the register allocator 
/// to accidentally overwrite the stack pointer.
BitVector RV32IRegisterInfo::getReservedRegs(MachineFunction const &MF) const {
  BitVector Reserved(getNumRegs());
  Reserved.set(RV32I::X0);
  Reserved.set(RV32I::X2);
  
  // If we have a dedicated frame pointer, we must reserve X8 as well.
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
  if (TFI->hasFP(MF))
    Reserved.set(RV32I::X8);
    
  return Reserved;
}

/// eliminateFrameIndex:
/// A very important pass! The compiler often generates instructions like:
///   `store a0, FrameIndex#2`
/// But the hardware doesn't know what "FrameIndex#2" is. It only understands 
/// memory addresses like `sp + offset`.
///
/// This function calculates the exact offset of `FrameIndex#2` from the stack 
/// pointer (or frame pointer), and transforms the instruction into:
///   `store a0, [sp + offset]`
bool RV32IRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                            int SPAdj,
                                            unsigned FIOperandNum,
                                            RegScavenger *RS) const {
  // Choice: Replace FrameIndex references with SP-relative addressing.
  // This is essential for correct code generation — without it, frame
  // index operands remain as placeholders and the output is invalid.
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetFrameLowering *TFL = MF.getSubtarget().getFrameLowering();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  int64_t Offset = MFI.getObjectOffset(FrameIndex) +
                   MFI.getStackSize() +
                   MI.getOperand(FIOperandNum + 1).getImm();

  // Determine the base register: use FP if available, otherwise SP.
  Register BaseReg = TFL->hasFP(MF) ? RV32I::X8 : RV32I::X2;

  MI.getOperand(FIOperandNum).ChangeToRegister(BaseReg, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
  return false;
}

Register RV32IRegisterInfo::getFrameRegister(MachineFunction const &MF) const {
  const TargetFrameLowering *TFL = MF.getSubtarget().getFrameLowering();
  // Choice: Use X8 (s0/fp) when FP is enabled, X2 (sp) otherwise.
  return TFL->hasFP(MF) ? RV32I::X8 : RV32I::X2;
}
