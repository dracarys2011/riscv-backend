//===-- RV32ISelLowering.cpp - RV32I DAG Lowering Implementation -*- C++ -*-===//
//
// This file implements the RV32ITargetLowering class. 
// It is heavily involved in the transition between machine-independent LLVM IR 
// and the machine-dependent SelectionDAG. 
//
// The two biggest responsibilities of this file are:
// 1. Defining what types and operations are legal on RV32I (in the constructor).
// 2. Implementing the C calling convention (LowerFormalArguments, LowerCall, 
//    LowerReturn) to manage how values are passed between functions.
//
//===----------------------------------------------------------------------===//

#include "RV32ISelLowering.h"
#include "RV32I.h"
#include "RV32ITargetMachine.h"
#include "RV32ISubtarget.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;
/// Constructor:
/// Here we configure the behavior of the SelectionDAG framework. We tell it 
/// which register classes to use for which data types, and what to do when it 
/// encounters operations that aren't natively supported (like dynamically sized allocas).
RV32ITargetLowering::RV32ITargetLowering(const TargetMachine &TM,
                                         const RV32ISubtarget &STI)
    : TargetLowering(TM, STI) {
  // Set up the register classes: For a 32-bit integer (i32), use the GPR class.
  addRegisterClass(MVT::i32, &RV32I::GPRRegClass);

  // Compute derived properties from the register classes.
  computeRegisterProperties(STI.getRegisterInfo());

  // SP is the stack pointer.
  setStackPointerRegisterToSaveRestore(RV32I::X2);

  // Set boolean contents to zero or one.
  setBooleanContents(ZeroOrOneBooleanContent);

  // Choice: Expand operations that RV32I base ISA cannot do natively.
  // RV32I has no hardware divide (that's M extension), no bit-manipulation
  // (B extension), no branch-on-CC, no SELECT_CC, etc.
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Expand);
  setOperationAction(ISD::SELECT, MVT::i32, Expand);
  
  // BRCOND: We handle this with Custom lowering, translating it into
  // a BNE against X0 (branch if the condition register != 0).
  setOperationAction(ISD::BRCOND, MVT::Other, Custom);
  
  // SETCC: We let LLVM keep this Legal. Our SLT/SLTU instructions directly
  // implement signed/unsigned less-than comparisons. Other conditions 
  // (EQ, NE, GT, GE) are synthesized by the legalizer using SLT + XOR.
  // This is the standard approach for RISC-V.
  setOperationAction(ISD::SETCC, MVT::i32, Legal);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);

  // No hardware multiply/divide in base RV32I.
  setOperationAction(ISD::MUL, MVT::i32, Expand);
  setOperationAction(ISD::SDIV, MVT::i32, Expand);
  setOperationAction(ISD::UDIV, MVT::i32, Expand);
  setOperationAction(ISD::SREM, MVT::i32, Expand);
  setOperationAction(ISD::UREM, MVT::i32, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::MULHU, MVT::i32, Expand);
  setOperationAction(ISD::MULHS, MVT::i32, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);
  setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);

  // No bit-manipulation.
  setOperationAction(ISD::CTTZ, MVT::i32, Expand);
  setOperationAction(ISD::CTLZ, MVT::i32, Expand);
  setOperationAction(ISD::CTPOP, MVT::i32, Expand);
  setOperationAction(ISD::ROTL, MVT::i32, Expand);
  setOperationAction(ISD::ROTR, MVT::i32, Expand);
  setOperationAction(ISD::BSWAP, MVT::i32, Expand);

  // Choice: Expand GlobalAddress to LUI+ADDI pair via custom lowering.
  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);

  // Expand dynamic stack allocation.
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32, Expand);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);
}

const char *RV32ITargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case RV32IISD::RET_FLAG: return "RV32IISD::RET_FLAG";
  case RV32IISD::CALL:     return "RV32IISD::CALL";
  case RV32IISD::BR_CC:    return "RV32IISD::BR_CC";
  default:                  return nullptr;
  }
}

SDValue RV32ITargetLowering::LowerOperation(SDValue Op,
                                            SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:
    Op.dump();
    llvm_unreachable("unimplemented operand");
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::BRCOND:
    return LowerBRCOND(Op, DAG);
  case ISD::BR_CC:
    return LowerBR_CC(Op, DAG);
  }
}

// Choice: Lower GlobalAddress using LUI + ADDI (absolute addressing).
// This is the simplest approach for static relocation model.
SDValue RV32ITargetLowering::LowerGlobalAddress(SDValue Op,
                                                 SelectionDAG &DAG) const {
  SDLoc DL(Op);
  EVT Ty = Op.getValueType();
  GlobalAddressSDNode *N = cast<GlobalAddressSDNode>(Op);
  int64_t Offset = N->getOffset();

  SDValue Hi = DAG.getTargetGlobalAddress(N->getGlobal(), DL, Ty, Offset,
                                          RV32III::MO_HI);
  SDValue Lo = DAG.getTargetGlobalAddress(N->getGlobal(), DL, Ty, Offset,
                                          RV32III::MO_LO);

  SDValue MNHi = SDValue(DAG.getMachineNode(RV32I::LUI, DL, Ty, Hi), 0);
  return SDValue(DAG.getMachineNode(RV32I::ADDI, DL, Ty, MNHi, Lo), 0);
}

#include "RV32IGenCallingConv.inc"

/// LowerBRCOND:
/// BRCOND takes (chain, condition, dest). In RISC-V, there's no single
/// "branch if register is true" instruction. Instead, we convert it to
/// BR_CC(SETNE, cond, X0, dest) — branch if condition != 0.
SDValue RV32ITargetLowering::LowerBRCOND(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Chain = Op.getOperand(0);
  SDValue Cond = Op.getOperand(1);
  SDValue Dest = Op.getOperand(2);
  SDValue Zero = DAG.getRegister(RV32I::X0, MVT::i32);
  // Build a BR_CC and redirect to our BR_CC lowering
  SDValue BRCC = DAG.getNode(ISD::BR_CC, DL, MVT::Other, Chain,
                              DAG.getCondCode(ISD::SETNE), Cond, Zero, Dest);
  return LowerBR_CC(BRCC, DAG);
}

/// LowerBR_CC:
/// BR_CC takes (chain, condcode, lhs, rhs, dest). We emit our custom
/// RV32IISD::BR_CC node: (chain, cc_int, lhs, rhs, dest).
/// The DAG-to-DAG selector will pick the correct BEQ/BNE/BLT/BGE/etc.
SDValue RV32ITargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue Dest = Op.getOperand(4);
  
  // For GT/LE conditions, swap operands to use LT/GE
  switch (CC) {
  default: break;
  case ISD::SETGT:  CC = ISD::SETLT;  std::swap(LHS, RHS); break;
  case ISD::SETLE:  CC = ISD::SETGE;  std::swap(LHS, RHS); break;
  case ISD::SETUGT: CC = ISD::SETULT; std::swap(LHS, RHS); break;
  case ISD::SETULE: CC = ISD::SETUGE; std::swap(LHS, RHS); break;
  }
  
  SDValue CCVal = DAG.getConstant(CC, DL, MVT::i32);
  return DAG.getNode(RV32IISD::BR_CC, DL, MVT::Other, Chain, CCVal, LHS, RHS, Dest);
}

/// LowerFormalArguments:
/// This function translates the incoming arguments of a function (the "formal" 
/// arguments) into SelectionDAG nodes.
/// 
/// When a function is called, the RISC-V ABI says arguments will be in 
/// registers `a0-a7` (X10-X17), and the rest on the stack. `CCState` (Calling 
/// Convention State) automatically analyzes this using the `CC_RV32I` definition 
/// from our TableGen files. We just have to read those assignments and generate 
/// `CopyFromReg` or `Load` nodes.
SDValue RV32ITargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_RV32I);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      EVT RegVT = VA.getLocVT();
      const unsigned VReg = RegInfo.createVirtualRegister(&RV32I::GPRRegClass);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      SDValue ArgIn = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);
      InVals.push_back(ArgIn);
    } else {
      assert(VA.isMemLoc());
      int FI = MF.getFrameInfo().CreateFixedObject(
          VA.getLocVT().getSizeInBits() / 8, VA.getLocMemOffset(), true);
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      SDValue Load = DAG.getLoad(
          VA.getValVT(), DL, Chain, FIN,
          MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI));
      InVals.push_back(Load);
    }
  }

  return Chain;
}

/// LowerReturn:
/// This function translates the `ret` IR instruction into the SelectionDAG.
/// It must take the value being returned, figure out which register it belongs 
/// in (usually `a0` / X10 according to `RetCC_RV32I`), generate a `CopyToReg` 
/// node, and finally attach our custom `RV32IISD::RET_FLAG` node at the end of 
/// the DAG to signal the return.
SDValue
RV32ITargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                 bool IsVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
                                 const SmallVectorImpl<SDValue> &OutVals,
                                 const SDLoc &DL, SelectionDAG &DAG) const {

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_RV32I);

  SDValue Glue;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Glue);
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain;
  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(RV32IISD::RET_FLAG, DL, MVT::Other, RetOps);
}

/// LowerCall:
/// This is the most complex of the ABI functions. When our code calls another 
/// function, we must:
/// 1. Analyze the outgoing arguments.
/// 2. Set up the stack (using `callseq_start`).
/// 3. Move the arguments into the correct registers (`a0-a7`) or push them 
///    onto the stack.
/// 4. Generate the actual `CALL` node.
/// 5. Clean up the stack (`callseq_end`).
/// 6. Retrieve the return value from `a0`.
SDValue RV32ITargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                       SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;

  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeCallOperands(Outs, CC_RV32I);

  unsigned NumBytes = CCInfo.getStackSize();
  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());
      SDValue StackPtr = DAG.getCopyFromReg(Chain, DL, RV32I::X2,
                                            getPointerTy(DAG.getDataLayout()));
      SDValue PtrOff = DAG.getIntPtrConstant(VA.getLocMemOffset(), DL);
      PtrOff = DAG.getNode(ISD::ADD, DL, getPointerTy(DAG.getDataLayout()),
                           StackPtr, PtrOff);
      MemOpChains.push_back(DAG.getStore(
          Chain, DL, Arg, PtrOff, MachinePointerInfo()));
    }
  }

  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  SDValue InGlue;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, DL, RegsToPass[i].first,
                             RegsToPass[i].second, InGlue);
    InGlue = Chain.getValue(1);
  }

  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL,
                                        getPointerTy(DAG.getDataLayout()));
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(),
                                         getPointerTy(DAG.getDataLayout()));

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  const uint32_t *Mask =
      MF.getSubtarget().getRegisterInfo()->getCallPreservedMask(MF, CallConv);
  assert(Mask && "Missing call preserved mask for calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (InGlue.getNode())
    Ops.push_back(InGlue);

  Chain = DAG.getNode(RV32IISD::CALL, DL, NodeTys, Ops);
  InGlue = Chain.getValue(1);

  Chain = DAG.getCALLSEQ_END(Chain, NumBytes, 0, InGlue, DL);
  if (!Ins.empty())
    InGlue = Chain.getValue(1);

  return LowerCallResult(Chain, InGlue, CallConv, IsVarArg, Ins, DL, DAG,
                         InVals);
}

SDValue RV32ITargetLowering::LowerCallResult(
    SDValue Chain, SDValue InGlue, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_RV32I);

  for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
    CCValAssign VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    // Choice: CopyFromReg returns a 3-result node: [value, chain, glue].
    // getValue(0) = the register value, getValue(1) = chain, getValue(2) = glue.
    SDValue Val = DAG.getCopyFromReg(Chain, DL, VA.getLocReg(),
                                     VA.getLocVT(), InGlue);
    Chain = Val.getValue(1);
    InGlue = Val.getValue(2);
    InVals.push_back(Val);
  }

  return Chain;
}
