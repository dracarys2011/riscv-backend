#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/TargetParser/Triple.h"
#include "llvm/Support/CodeGen.h"

using namespace llvm;

// Register positional argument so LLVM's CommandLine parser knows about it.
static cl::opt<std::string> InputFilename(cl::Positional,
                                           cl::desc("<input .ll file>"),
                                           cl::Required);

extern "C" void LLVMInitializeRV32ITargetInfo();
extern "C" void LLVMInitializeRV32ITarget();
extern "C" void LLVMInitializeRV32ITargetMC();
extern "C" void LLVMInitializeRV32IAsmPrinter();

int main(int argc, char **argv) {
  // Parse LLVM command line options (this enables -print-after=, -debug, etc.)
  cl::ParseCommandLineOptions(argc, argv, "RV32I Compiler\n");

  // Initialize our custom target
  LLVMInitializeRV32ITargetInfo();
  LLVMInitializeRV32ITarget();
  LLVMInitializeRV32ITargetMC();
  LLVMInitializeRV32IAsmPrinter();

  LLVMContext Context;
  SMDiagnostic Err;
  std::unique_ptr<Module> M = parseIRFile(InputFilename, Err, Context);
  if (!M) {
    Err.print(argv[0], errs());
    return 1;
  }

  std::string Error;
  std::string TargetTripleStr = "rv32i-unknown-elf";
  Triple TT(TargetTripleStr);
  const Target *TheTarget = TargetRegistry::lookupTarget(TargetTripleStr, Error);
  if (!TheTarget) {
    errs() << Error;
    return 1;
  }

  TargetOptions opt;
  std::unique_ptr<TargetMachine> TargetMachine(
      TheTarget->createTargetMachine(TT, "", "", opt, std::nullopt));

  M->setDataLayout(TargetMachine->createDataLayout());
  M->setTargetTriple(TT);

  std::error_code EC;
  ToolOutputFile Out("output.s", EC, sys::fs::OF_None);
  if (EC) {
    errs() << EC.message() << '\n';
    return 1;
  }

  legacy::PassManager Pass;
  if (TargetMachine->addPassesToEmitFile(Pass, Out.os(), nullptr, CodeGenFileType::AssemblyFile)) {
    errs() << "TargetMachine can't emit a file of this type\n";
    return 1;
  }

  Pass.run(*M);
  Out.keep();
  
  outs() << "Successfully compiled to output.s\n";
  return 0;
}
