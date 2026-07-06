//===-- RV32IInstrInfo.h - RV32I Instruction Information --------*- C++ -*-===//
//
// This file implements the TargetInstrInfo interface. It is used to interact 
// with the machine instructions generated from the SelectionDAG. 
// Key responsibilities include:
// - Copying physical registers (e.g., when passing arguments).
// - Storing/Loading registers to/from stack slots (called "spilling").
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RV32I_RV32IINSTRINFO_H
#define LLVM_LIB_TARGET_RV32I_RV32IINSTRINFO_H

#include "RV32IRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_ENUM
#include "RV32IGenInstrInfo.inc"

#define GET_INSTRINFO_HEADER
#include "RV32IGenInstrInfo.inc"

namespace llvm {

class RV32ISubtarget;

/// RV32IInstrInfo:
/// Inherits from the TableGen auto-generated `RV32IGenInstrInfo`.
class RV32IInstrInfo : public RV32IGenInstrInfo {
  const RV32IRegisterInfo RI;
  const RV32ISubtarget &STI;

public:
  RV32IInstrInfo(const RV32ISubtarget &STI);

  const RV32IRegisterInfo &getRegisterInfo() const { return RI; }

  /// copyPhysReg:
  /// Emits an instruction to copy a value from one physical register to another.
  /// E.g., `addi a0, a1, 0`.
  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                   const DebugLoc &DL, Register DstReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  /// storeRegToStackSlot / loadRegFromStackSlot:
  /// Whenever the register allocator runs out of physical registers (register pressure),
  /// it will "spill" a register to the stack using `storeRegToStackSlot` and later 
  /// "reload" it using `loadRegFromStackSlot`.
  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI, Register SrcReg,
                           bool IsKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           Register VReg,
                           MachineInstr::MIFlag Flags =
                               MachineInstr::NoFlags) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI, Register DstReg,
                            int FrameIndex, const TargetRegisterClass *RC,
                            Register VReg, unsigned SubReg = 0,
                            MachineInstr::MIFlag Flags =
                                MachineInstr::NoFlags) const override;

  // Choice: LLVM 23 changed return type from unsigned to Register.
  Register isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;
  Register isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_RV32I_RV32IINSTRINFO_H
