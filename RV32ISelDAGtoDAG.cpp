//===-- RV32ISelDAGtoDAG.cpp - A dag to dag inst selector for RV32I -*- C++ -*-===//
//
// Here is the actual implementation of the instruction selector. 
// For this basic backend, we mostly rely on the auto-generated matcher 
// (`SelectCode`), but we provide the override `Select` in case we ever 
// need to manually match complex patterns that TableGen can't handle.
//
//===----------------------------------------------------------------------===//

#include "RV32ISelDAGtoDAG.h"
#include "RV32I.h"
#include "RV32ISelLowering.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "rv32i-isel"
#define PASS_NAME "RV32I DAG->DAG Pattern Instruction Selection"

/// Select:
/// This method is called for every node in the SelectionDAG.
/// For most nodes, we fall through to the auto-generated `SelectCode` which
/// uses the patterns from our `.td` files. But some nodes like `FrameIndex`
/// require manual handling here because they don't have a direct TableGen
/// pattern — they represent abstract stack slot references that the register
/// allocator will resolve later.
void RV32IDAGToDAGISel::Select(SDNode *Node) {
  // If the node is already a machine opcode (selected), we just skip it.
  if (Node->isMachineOpcode()) {
    LLVM_DEBUG(dbgs() << "== "; Node->dump(CurDAG); dbgs() << "\n");
    Node->setNodeId(-1);
    return;
  }

  switch (Node->getOpcode()) {
  default:
    break;
  case ISD::FrameIndex: {
    // A FrameIndex node represents a reference to a stack slot (from alloca).
    // We convert it to: ADDI rd, FrameIndex, 0
    // Later, eliminateFrameIndex() in RV32IRegisterInfo will replace the
    // FrameIndex operand with the actual SP + offset.
    SDLoc DL(Node);
    int FI = cast<FrameIndexSDNode>(Node)->getIndex();
    SDValue TFI = CurDAG->getTargetFrameIndex(FI, MVT::i32);
    SDValue Imm = CurDAG->getTargetConstant(0, DL, MVT::i32);
    ReplaceNode(Node,
                CurDAG->getMachineNode(RV32I::ADDI, DL, MVT::i32, TFI, Imm));
    return;
  }
  case RV32IISD::BR_CC: {
    // RV32IISD::BR_CC: (chain, cc_int, lhs, rhs, dest)
    // Select the appropriate branch instruction based on the condition code.
    SDLoc DL(Node);
    SDValue Chain = Node->getOperand(0);
    int CC = cast<ConstantSDNode>(Node->getOperand(1))->getSExtValue();
    SDValue LHS = Node->getOperand(2);
    SDValue RHS = Node->getOperand(3);
    SDValue Dest = Node->getOperand(4);
    
    unsigned Opc;
    switch (static_cast<ISD::CondCode>(CC)) {
    default: llvm_unreachable("Unexpected condition code in BR_CC");
    case ISD::SETEQ:  Opc = RV32I::BEQ; break;
    case ISD::SETNE:  Opc = RV32I::BNE; break;
    case ISD::SETLT:  Opc = RV32I::BLT; break;
    case ISD::SETGE:  Opc = RV32I::BGE; break;
    case ISD::SETULT: Opc = RV32I::BLTU; break;
    case ISD::SETUGE: Opc = RV32I::BGEU; break;
    }
    
    SmallVector<SDValue, 4> Ops = {LHS, RHS, Dest, Chain};
    CurDAG->SelectNodeTo(Node, Opc, MVT::Other, Ops);
    return;
  }
  }

  // Fallback to the auto-generated matcher from TableGen.
  // It will automatically match nodes like `add` to `ADD` based on our `.td` rules.
  SelectCode(Node);
}

// Choice: LLVM 23 Legacy pass wrapper implementation.
char RV32IDAGToDAGISelLegacy::ID = 0;

/// Wrapper Constructor:
/// We instantiate our actual ISel logic (`RV32IDAGToDAGISel`) inside this 
/// wrapper so it can interface with the legacy pass manager.
RV32IDAGToDAGISelLegacy::RV32IDAGToDAGISelLegacy(RV32ITargetMachine &TM,
                                                   CodeGenOptLevel OptLevel)
    : SelectionDAGISelLegacy(
          ID, std::make_unique<RV32IDAGToDAGISel>(TM, OptLevel)) {}

/// createRV32IISelDag:
/// Factory function called by `addInstSelector` in our TargetMachine.
FunctionPass *llvm::createRV32IISelDag(RV32ITargetMachine &TM,
                                       CodeGenOptLevel OptLevel) {
  return new RV32IDAGToDAGISelLegacy(TM, OptLevel);
}
