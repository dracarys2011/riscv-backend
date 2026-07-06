//===-- RV32ITargetObjectFile.h - RV32I Object Info -------------*- C++ -*-===//
//
// When LLVM compiles code, it eventually needs to place instructions and data 
// into specific "sections" of an object file (like `.text` for code, `.data` 
// for initialized variables, `.bss` for uninitialized variables, etc.).
// 
// This class tells LLVM how to manage those sections for the RV32I architecture
// using the ELF file format.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RV32I_RV32ITARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_RV32I_RV32ITARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {
class RV32ITargetMachine;

/// RV32IELFTargetObjectFile:
/// We inherit from TargetLoweringObjectFileELF because RISC-V typically 
/// uses the ELF (Executable and Linkable Format) standard for its object files.
class RV32IELFTargetObjectFile : public TargetLoweringObjectFileELF {
  /// Initialize:
  /// Called to set up section names and attributes when the TargetMachine is created.
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;
};
} // end namespace llvm

#endif
