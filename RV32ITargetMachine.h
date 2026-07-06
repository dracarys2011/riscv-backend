//===-- RV32ITargetMachine.h - Define TargetMachine for RV32I ---*- C++ -*-===//
//
// Welcome to the TargetMachine!
// The TargetMachine is the primary point of contact between LLVM's core 
// infrastructure and our custom RV32I backend. When you invoke `llc` or run 
// the JIT, LLVM asks the TargetMachine to coordinate the code generation process.
//
// It holds instances of critical components, such as the Subtarget (which 
// defines the CPU's specific features and registers), and it configures the 
// pass pipeline that will turn LLVM IR into RISC-V assembly.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_RV32I_TARGET_MACHINE_H
#define LLVM_RV32I_TARGET_MACHINE_H

#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "RV32ISubtarget.h"
#include <memory>
#include <optional>

namespace llvm {

// Forward declarations to avoid including heavy headers in this file.
class RV32ISubtarget;
class Function;
class PassManagerBase;
class TargetPassConfig;

/// RV32ITargetMachine - This class represents our specific RV32I target.
/// It inherits from CodeGenTargetMachineImpl, which provides the base 
/// implementations for generic LLVM code generation workflows.
class RV32ITargetMachine : public CodeGenTargetMachineImpl {
  /// TargetLoweringObjectFile (TLOF) determines how globals and sections 
  /// (like .text, .data, .bss) are emitted to the final object file (ELF in our case).
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  
  /// The Subtarget contains properties specific to a particular variant of 
  /// the architecture. Even though we just have a basic RV32I, LLVM requires 
  /// a Subtarget to manage things like register files, instruction info, and lowering.
  RV32ISubtarget Subtarget;

public:
  /// Constructor: Initializes the target machine with the target triple, CPU, 
  /// features, and code generation options (like Relocation Model and Code Model).
  RV32ITargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     std::optional<Reloc::Model> RM,
                     std::optional<CodeModel::Model> CM,
                     CodeGenOptLevel OL, bool JIT);

  ~RV32ITargetMachine() override;

  /// getSubtargetImpl: Given an LLVM Function, this returns the subtarget 
  /// for that function. This allows for function-level subtarget features 
  /// (e.g. compiling one function with standard features and another with extensions).
  const TargetSubtargetInfo *getSubtargetImpl(const Function &F) const override;

  /// createPassConfig: This is where we plug our backend into the LLVM pass manager!
  /// We return a TargetPassConfig object, which we use to insert our custom 
  /// instruction selection pass, prologue/epilogue insertion, and other 
  /// backend-specific optimization passes.
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  /// getObjFileLowering: Returns our TargetLoweringObjectFile instance so LLVM 
  /// knows how to emit sections for our chosen object format (ELF).
  TargetLoweringObjectFile *getObjFileLowering() const override;
};

} // namespace llvm

#endif // LLVM_RV32I_TARGET_MACHINE_H
