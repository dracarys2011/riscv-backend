//===-- RV32IAsmPrinter.h - RV32I LLVM Assembly Printer ---------*- C++ -*-===//
//
// The AsmPrinter is the final step in the main CodeGen pipeline! 
// It takes the fully formed, register-allocated `MachineInstr`s and translates 
// them into `MCInst`s (Machine Code Instructions).
//
// Once it creates an `MCInst`, it passes it to the `MCStreamer` which will 
// either print it as text (`.s` file) or encode it as binary (`.o` object file).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RV32I_RV32IASMPRINTER_H
#define LLVM_LIB_TARGET_RV32I_RV32IASMPRINTER_H

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
class TargetMachine;

/// RV32IAsmPrinter:
/// Inherits from AsmPrinter to provide custom logic for RV32I.
class LLVM_LIBRARY_VISIBILITY RV32IAsmPrinter : public AsmPrinter {
public:
  explicit RV32IAsmPrinter(TargetMachine &TM,
                           std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  StringRef getPassName() const override { return "RV32I Assembly Printer"; }

  /// emitInstruction:
  /// This is where the translation from `MachineInstr` to `MCInst` happens.
  void emitInstruction(const MachineInstr *MI) override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_RV32I_RV32IASMPRINTER_H
