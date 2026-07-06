//===-- RV32ISelLowering.h - RV32I DAG Lowering Interface -----*- C++ -*-===//
//
// TargetLowering is one of the most important interfaces in the backend!
// It tells LLVM's generic SelectionDAG which operations are "Legal" 
// (supported directly by hardware) and which need to be "Expanded" (emulated) 
// or "Custom" lowered (handled manually by our C++ code).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RV32I_RV32ISELLOWERING_H
#define LLVM_LIB_TARGET_RV32I_RV32ISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/SelectionDAG.h"

namespace llvm {
class RV32ISubtarget;

/// RV32IISD:
/// Custom node types specific to RV32I. We use these when standard LLVM ISD 
/// nodes don't perfectly match our hardware semantics (like returns and calls).
namespace RV32IISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET_FLAG,
  CALL,
  BR_CC  // Custom conditional branch: (chain, cc, lhs, rhs, dest)
};
} // end namespace RV32IISD

// Choice: Target-specific flags for relocations (used in GlobalAddress lowering).
namespace RV32III {
enum {
  MO_None = 0,
  MO_HI = 1,   // %hi(symbol) — upper 20 bits for LUI
  MO_LO = 2,   // %lo(symbol) — lower 12 bits for ADDI
};
} // end namespace RV32III

/// RV32ITargetLowering:
/// The primary class for defining lowering behaviors.
class RV32ITargetLowering : public TargetLowering {
public:
  explicit RV32ITargetLowering(const TargetMachine &TM,
                               const RV32ISubtarget &STI);

  /// getTargetNodeName: Returns the string representation of our custom RV32IISD nodes.
  const char *getTargetNodeName(unsigned Opcode) const override;

  /// LowerOperation:
  /// This is the entry point for custom lowering. If we told LLVM an operation 
  /// was "Custom", LLVM calls this function to ask us how to translate the node.
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  /// LowerFormalArguments:
  /// How are arguments passed *into* our functions? (Registers? Stack?)
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  /// LowerReturn:
  /// How does our function return a value back to the caller?
  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals,
                      const SDLoc &DL, SelectionDAG &DAG) const override;

  /// LowerCall:
  /// How do we set up the arguments and make a function call?
  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerCallResult(SDValue Chain, SDValue InGlue,
                          CallingConv::ID CallConv, bool IsVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins,
                          const SDLoc &DL, SelectionDAG &DAG,
                          SmallVectorImpl<SDValue> &InVals) const;

private:
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBRCOND(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_RV32I_RV32ISELLOWERING_H
