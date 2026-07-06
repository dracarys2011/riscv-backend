//===-- RV32I.td - Describe the RV32I Target Machine ---------*- tablegen -*-===//
//
// Welcome to TableGen! 
// LLVM uses TableGen (a DSL) to describe the architecture abstractly. From 
// these `.td` files, the `llvm-tblgen` tool auto-generates massive amounts 
// of C++ code for registers, instructions, calling conventions, and selection 
// logic, saving us from writing thousands of lines of boilerplate.
//
// This is the top-level file for the RV32I target. It simply includes all 
// the other component definitions and ties them together into the final 
// `Target` definition.
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

// -----------------------------------------------------------------------------
// Subtarget Features
// -----------------------------------------------------------------------------

// (None for the initial RV32I backend, but this is where we'd define 
// things like FeatureM for the multiplication extension)

// -----------------------------------------------------------------------------
// Register and Instruction Definitions
// -----------------------------------------------------------------------------

include "RV32IRegisterInfo.td"

// -----------------------------------------------------------------------------
// Calling Convention
// -----------------------------------------------------------------------------

include "RV32ICallingConv.td"

include "RV32IInstrFormats.td"
include "RV32IInstrInfo.td"

// -----------------------------------------------------------------------------
// Processor Models
// -----------------------------------------------------------------------------

include "RV32ISchedule.td"

/// ProcessorModel: Defines a generic CPU that supports our features.
def : ProcessorModel<"generic-rv32", GenericRV32Model, []>;

/// InstrInfo: The wrapper connecting our instruction set definitions.
def RV32IInstrInfo : InstrInfo;

/// AsmWriter: Instructs LLVM how to print assembly strings (e.g. knowing 
/// that `add x1, x2, x3` shouldn't be printed as `x1 add x2 x3`).
def RV32IAsmWriter : AsmWriter {
  int Variant = 0;
  int PassSubtarget = 1;
}

/// Target: The grand finale! This maps our components to the actual LLVM Target.
def RV32I : Target {
  let InstructionSet = RV32IInstrInfo;
  let AssemblyWriters = [RV32IAsmWriter];
}

/// Remap pseudo pointers (since we are a 32-bit architecture, pointers are in GPRs).
defm : RemapAllTargetPseudoPointerOperands<GPR>;