//===-- RV32ITargetMachine.cpp - Define TargetMachine for RV32I -*- C++ -*-===//
//
// Welcome to the TargetMachine Implementation!
// Here, we provide the concrete C++ implementation of our TargetMachine class.
// This file initializes the TargetMachine, sets up the DataLayout, and
// tells the LLVM Pass Manager which passes to run (like Instruction Selection).
//
//===----------------------------------------------------------------------===//

#include "RV32ITargetMachine.h"
#include "RV32ITargetInfo.h"
#include "RV32ITargetObjectFile.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

/// Initialization Function:
/// When LLVM loads our target (or when our standalone compiler driver initializes it),
/// it calls `LLVMInitializeRV32ITarget()`. Here, we register our RV32ITargetMachine
/// class with the TargetRegistry, associating it with our specific target architecture.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRV32ITarget() {
  RegisterTargetMachine<RV32ITargetMachine> X(getTheRV32ITarget());
}

static std::string computeDataLayout(const Triple &TT) {
  // Let's break it down:
  // e        : little-endian
  // m:e      : ELF mangling
  // p:32:32  : Pointers are 32 bits wide, and their natural alignment is 32 bits
  // i64:64   : 64-bit integers should be 64-bit aligned (per RISC-V ABI)
  // n32      : Native integer width is 32 bits
  // S128     : Stack is 128-bit (16-byte) aligned
  return "e-m:e-p:32:32-i64:64-n32-S128";
}

/// Constructor:
/// We pass our layout string, CPU, features, and various options down to 
/// CodeGenTargetMachineImpl. We also instantiate our TLOF (for ELF object files)
/// and our Subtarget. Finally, `initAsmInfo()` fetches formatting options for assembly.
RV32ITargetMachine::RV32ITargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       std::optional<Reloc::Model> RM,
                                       std::optional<CodeModel::Model> CM,
                                       CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, computeDataLayout(TT), TT, CPU, FS, Options,
                               RM.value_or(Reloc::Static),
                               CM.value_or(CodeModel::Small), OL),
      TLOF(std::make_unique<RV32IELFTargetObjectFile>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}

RV32ITargetMachine::~RV32ITargetMachine() = default;

/// getSubtargetImpl:
/// Since we don't support function-specific attributes in this basic backend, 
/// we just return a pointer to our single global Subtarget instance.
const TargetSubtargetInfo *RV32ITargetMachine::getSubtargetImpl(const Function &F) const {
  return &Subtarget;
}

namespace {
/// TargetPassConfig Implementation:
/// This class configures the pipeline of passes used to generate machine code.
class RV32IPassConfig : public TargetPassConfig {
public:
  RV32IPassConfig(RV32ITargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  RV32ITargetMachine &getRV32ITargetMachine() const {
    return getTM<RV32ITargetMachine>();
  }

  /// addInstSelector:
  /// This is the most crucial pass we must provide! It translates LLVM 
  /// SelectionDAG nodes into RV32I MachineInstr nodes. If we don't add this, 
  /// LLVM won't know how to select instructions for our target.
  bool addInstSelector() override {
    addPass(createRV32IISelDag(getRV32ITargetMachine(), getOptLevel()));
    return false;
  }
};
} // namespace

/// createPassConfig:
/// Hook to instantiate our custom RV32IPassConfig.
TargetPassConfig *RV32ITargetMachine::createPassConfig(PassManagerBase &PM) {
  return new RV32IPassConfig(*this, PM);
}

/// getObjFileLowering:
/// Returns our TargetLoweringObjectFile which we initialized in the constructor.
TargetLoweringObjectFile *RV32ITargetMachine::getObjFileLowering() const {
  return TLOF.get();
}

