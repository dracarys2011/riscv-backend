//===-- RV32IInstrInfo.cpp - RV32I Instruction Information --------*- C++ -*-===//
//
// This file implements the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "RV32IInstrInfo.h"
#include "RV32I.h"
#include "RV32ISubtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "RV32IGenInstrInfo.inc"

using namespace llvm;

// Choice: LLVM 23 changed RV32IGenInstrInfo constructor to require
// (STI, TRI, CFSetup, CFDestroy, CatchRet, Return). We pass the
// Subtarget and its RegisterInfo, plus our call frame pseudos.
RV32IInstrInfo::RV32IInstrInfo(const RV32ISubtarget &Subtarget)
    : RV32IGenInstrInfo(Subtarget, Subtarget.getRegisterInfo()
                            ? *Subtarget.getRegisterInfo()
                            : *static_cast<const TargetRegisterInfo *>(nullptr),
                        RV32I::ADJCALLSTACKDOWN, RV32I::ADJCALLSTACKUP),
      RI(Subtarget), STI(Subtarget) {}

/// copyPhysReg:
/// The register allocator or the DAG will frequently ask us to copy the value
/// from one physical register to another. In RISC-V, we accomplish this by 
/// adding 0 to the source register and storing the result in the destination 
/// register (i.e., `addi dst, src, 0`).
void RV32IInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MBBI,
                                 const DebugLoc &DL, Register DstReg,
                                 Register SrcReg, bool KillSrc,
                                 bool RenamableDest,
                                 bool RenamableSrc) const {
  // Choice: Use ADDI rd, rs, 0 to implement register-to-register copy
  // (MV pseudo in standard RISC-V).
  if (RV32I::GPRRegClass.contains(DstReg) &&
      RV32I::GPRRegClass.contains(SrcReg)) {
    BuildMI(MBB, MBBI, DL, get(RV32I::ADDI), DstReg)
        .addReg(SrcReg, getKillRegState(KillSrc))
        .addImm(0);
    return;
  }

  llvm_unreachable("Impossible reg-to-reg copy");
}

/// storeRegToStackSlot:
/// Spills a register to a memory location on the stack.
/// We build an SW (Store Word) instruction, setting its destination as the 
/// abstract FrameIndex, and its source as the register to spill.
void RV32IInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator I,
                                         Register SrcReg, bool IsKill,
                                         int FI,
                                         const TargetRegisterClass *RC,
                                         Register VReg,
                                         MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  if (RV32I::GPRRegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, I, DL, get(RV32I::SW))
        .addReg(SrcReg, getKillRegState(IsKill))
        .addFrameIndex(FI)
        .addImm(0)
        .setMIFlag(Flags);
    return;
  }

  llvm_unreachable("Can't store this register to stack slot");
}

/// loadRegFromStackSlot:
/// Reloads a register from a memory location on the stack.
/// We build an LW (Load Word) instruction, setting its destination as the 
/// register to reload into, and its source as the abstract FrameIndex.
void RV32IInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator I,
                                          Register DstReg, int FI,
                                          const TargetRegisterClass *RC,
                                          Register VReg, unsigned SubReg,
                                          MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  if (RV32I::GPRRegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, I, DL, get(RV32I::LW), DstReg)
        .addFrameIndex(FI)
        .addImm(0)
        .setMIFlag(Flags);
    return;
  }

  llvm_unreachable("Can't load this register from stack slot");
}

Register RV32IInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                             int &FrameIndex) const {
  if (MI.getOpcode() == RV32I::LW) {
    if (MI.getOperand(1).isFI() && MI.getOperand(2).isImm() &&
        MI.getOperand(2).getImm() == 0) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
  }
  return Register();
}

Register RV32IInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                            int &FrameIndex) const {
  if (MI.getOpcode() == RV32I::SW) {
    if (MI.getOperand(1).isFI() && MI.getOperand(2).isImm() &&
        MI.getOperand(2).getImm() == 0) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
  }
  return Register();
}
