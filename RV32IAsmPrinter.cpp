//===-- RV32IAsmPrinter.cpp - RV32I LLVM Assembly Printer -------*- C++ -*-===//
//
// This file contains the implementation of the RV32IAsmPrinter.
// Its main job is to convert `MachineInstr` to `MCInst`.
//
//===----------------------------------------------------------------------===//

#include "RV32IAsmPrinter.h"
#include "RV32I.h"
#include "RV32ITargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

/// LowerRV32IMachineInstrToMCInst:
/// A helper function that takes a `MachineInstr` (MI) and builds an `MCInst`.
/// It iterates over all the operands (registers, immediates, globals) of the MI 
/// and converts them into MCOperands.
static void LowerRV32IMachineInstrToMCInst(const MachineInstr *MI,
                                           MCInst &OutMI,
                                           const AsmPrinter &AP) {
  OutMI.setOpcode(MI->getOpcode());
  
  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    switch (MO.getType()) {
    default:
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_RegisterMask:
      continue;
    case MachineOperand::MO_Register:
      if (MO.isImplicit()) continue;
      MCOp = MCOperand::createReg(MO.getReg());
      break;
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    case MachineOperand::MO_MachineBasicBlock:
      MCOp = MCOperand::createExpr(MCSymbolRefExpr::create(
          MO.getMBB()->getSymbol(), AP.OutContext));
      break;
    case MachineOperand::MO_GlobalAddress:
      MCOp = MCOperand::createExpr(MCSymbolRefExpr::create(
          AP.getSymbol(MO.getGlobal()), AP.OutContext));
      break;
    }
    OutMI.addOperand(MCOp);
  }
}

/// emitInstruction:
/// Called by the standard LLVM AsmPrinter pass for every instruction in the 
/// function. We lower it to an MCInst and pass it to the OutStreamer.
void RV32IAsmPrinter::emitInstruction(const MachineInstr *MI) {
  MCInst TmpInst;
  LowerRV32IMachineInstrToMCInst(MI, TmpInst, *this);
  EmitToStreamer(*OutStreamer, TmpInst);
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeRV32IAsmPrinter() {
  RegisterAsmPrinter<RV32IAsmPrinter> X(getTheRV32ITarget());
}
