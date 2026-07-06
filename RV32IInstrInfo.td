//===-- RV32IInstrInfo.td - Target Description for RV32I ---*- tablegen -*-===//
//
// Welcome to the core of the RISC-V backend!
// This file declares the actual instructions (ADD, SUB, LW, SW, etc.).
// It also maps LLVM IR operations (like `add` or `store`) to these instructions
// using DAG matching patterns.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// RV32I specific DAG Nodes.
//===----------------------------------------------------------------------===//
/// SDNode definitions allow us to create custom nodes in the SelectionDAG.
/// For example, we define `RV32IRetFlag` to represent our custom return operation.

// Define a return node.
def RV32IRetFlag : SDNode<"RV32IISD::RET_FLAG", SDTNone,
                          [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def RV32ICall : SDNode<"RV32IISD::CALL", SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>,
                       [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;

//===----------------------------------------------------------------------===//
// Operands and Types
//===----------------------------------------------------------------------===//
/// Operands define the types of arguments instructions can take.
/// `simm12` represents a 12-bit signed immediate. The `ImmLeaf` predicate 
/// tells the instruction selector: "Only match this pattern if the immediate 
/// fits within 12 bits signed".

// 12-bit signed immediate (for I-Type and S-Type)
def simm12 : Operand<i32>, ImmLeaf<i32, [{ return isInt<12>(Imm); }]> {
  let PrintMethod = "printSimm12Operand";
  let DecoderMethod = "decodeSImmOperand<12>";
}

// 13-bit branch target (for B-Type)
def brtarget : Operand<OtherVT> {
  let PrintMethod = "printBranchOperand";
  let EncoderMethod = "getBranchTargetOpValue";
  let DecoderMethod = "decodeBranchTarget";
}

// 21-bit jump target (for J-Type)
def jmptarget : Operand<i32> {
  let PrintMethod = "printBranchOperand";
  let EncoderMethod = "getBranchTargetOpValue";
  let DecoderMethod = "decodeJmpTarget";
}

def jmptarget_bb : Operand<OtherVT> {
  let PrintMethod = "printBranchOperand";
  let EncoderMethod = "getBranchTargetOpValue";
  let DecoderMethod = "decodeJmpTarget";
}

// 20-bit upper immediate (for U-Type)
def uimm20 : Operand<i32>, ImmLeaf<i32, [{ return isUInt<20>(Imm); }]> {
  let PrintMethod = "printUImm20Operand";
  let DecoderMethod = "decodeUImm20";
}

// Shift amount (5 bits)
def shamt : Operand<i32>, ImmLeaf<i32, [{ return isUInt<5>(Imm); }]>;

//===----------------------------------------------------------------------===//
// Pseudo Instructions
//===----------------------------------------------------------------------===//
/// Pseudo-instructions don't exist in the actual RISC-V hardware. 
/// They are temporary placeholders used by the compiler (e.g. during frame 
/// lowering or call setup) that get expanded into real instructions later.




def SDT_RV32ICallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_RV32ICallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_RV32ICallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_RV32ICallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

let Defs = [X2], Uses = [X2], hasSideEffects = 1, mayLoad = 0, mayStore = 0 in {
  def ADJCALLSTACKDOWN : RV32IPseudo<(outs), (ins i32imm:$amt, i32imm:$amt2),
                                     "#ADJCALLSTACKDOWN $amt",
                                     [(callseq_start timm:$amt, timm:$amt2)]>;
  def ADJCALLSTACKUP   : RV32IPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                     "#ADJCALLSTACKUP $amt1",
                                     [(callseq_end timm:$amt1, timm:$amt2)]>;
}

// Return pseudo instruction (lowered to JALR later)
let isReturn = 1, isTerminator = 1, isBarrier = 1, Uses = [X1] in
def PseudoRET : RV32IPseudo<(outs), (ins), "ret", [(RV32IRetFlag)]>;

let isCall = 1, Defs = [X1], Uses = [X2] in
def PseudoCALL : RV32IPseudo<(outs), (ins jmptarget:$dst), "call\t$dst", [(RV32ICall tglobaladdr:$dst)]>;

def : Pat<(RV32ICall texternalsym:$dst), (PseudoCALL texternalsym:$dst)>;

//===----------------------------------------------------------------------===//
// Instruction definitions & Pattern Matching
//===----------------------------------------------------------------------===//
/// This is where the magic happens!
/// By defining `ADD` as an `RVInstR`, we automatically get the 32-bit encoding.
/// Furthermore, the last argument `[(set GPR:$rd, (add GPR:$rs1, GPR:$rs2))]` 
/// is a DAG pattern. It tells the SelectionDAG: 
/// "When you see an `add` node with two GPRs, replace it with this ADD instruction!"

// R-Type ALUs
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
  def ADD  : RVInstR<0b0000000, 0b000, 0b0110011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2), "add\t$rd, $rs1, $rs2", [(set GPR:$rd, (add GPR:$rs1, GPR:$rs2))]>;
  def SUB  : RVInstR<0b0100000, 0b000, 0b0110011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2), "sub\t$rd, $rs1, $rs2", [(set GPR:$rd, (sub GPR:$rs1, GPR:$rs2))]>;
  def SLL  : RVInstR<0b0000000, 0b001, 0b0110011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2), "sll\t$rd, $rs1, $rs2", [(set GPR:$rd, (shl GPR:$rs1, GPR:$rs2))]>;
  def SLT  : RVInstR<0b0000000, 0b010, 0b0110011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2), "slt\t$rd, $rs1, $rs2", [(set GPR:$rd, (setcc GPR:$rs1, GPR:$rs2, SETLT))]>;
  def SLTU : RVInstR<0b0000000, 0b011, 0b0110011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2), "sltu\t$rd, $rs1, $rs2", [(set GPR:$rd, (setcc GPR:$rs1, GPR:$rs2, SETULT))]>;
  def XOR  : RVInstR<0b0000000, 0b100, 0b0110011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2), "xor\t$rd, $rs1, $rs2", [(set GPR:$rd, (xor GPR:$rs1, GPR:$rs2))]>;
  def SRL  : RVInstR<0b0000000, 0b101, 0b0110011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2), "srl\t$rd, $rs1, $rs2", [(set GPR:$rd, (srl GPR:$rs1, GPR:$rs2))]>;
  def SRA  : RVInstR<0b0100000, 0b101, 0b0110011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2), "sra\t$rd, $rs1, $rs2", [(set GPR:$rd, (sra GPR:$rs1, GPR:$rs2))]>;
  def OR   : RVInstR<0b0000000, 0b110, 0b0110011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2), "or\t$rd, $rs1, $rs2",  [(set GPR:$rd, (or GPR:$rs1, GPR:$rs2))]>;
  def AND  : RVInstR<0b0000000, 0b111, 0b0110011, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2), "and\t$rd, $rs1, $rs2", [(set GPR:$rd, (and GPR:$rs1, GPR:$rs2))]>;
}

// I-Type ALUs
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
  def ADDI  : RVInstI<0b000, 0b0010011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "addi\t$rd, $rs1, $imm12", [(set GPR:$rd, (add GPR:$rs1, simm12:$imm12))]>;
  def SLTI  : RVInstI<0b010, 0b0010011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "slti\t$rd, $rs1, $imm12", [(set GPR:$rd, (setcc GPR:$rs1, simm12:$imm12, SETLT))]>;
  def SLTIU : RVInstI<0b011, 0b0010011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "sltiu\t$rd, $rs1, $imm12", [(set GPR:$rd, (setcc GPR:$rs1, simm12:$imm12, SETULT))]>;
  def XORI  : RVInstI<0b100, 0b0010011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "xori\t$rd, $rs1, $imm12", [(set GPR:$rd, (xor GPR:$rs1, simm12:$imm12))]>;
  def ORI   : RVInstI<0b110, 0b0010011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "ori\t$rd, $rs1, $imm12",  [(set GPR:$rd, (or GPR:$rs1, simm12:$imm12))]>;
  def ANDI  : RVInstI<0b111, 0b0010011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "andi\t$rd, $rs1, $imm12", [(set GPR:$rd, (and GPR:$rs1, simm12:$imm12))]>;
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
  def SLLI : RVInstIShift<0b0000000, 0b001, 0b0010011, (outs GPR:$rd), (ins GPR:$rs1, shamt:$shamt), "slli\t$rd, $rs1, $shamt", [(set GPR:$rd, (shl GPR:$rs1, shamt:$shamt))]>;
  def SRLI : RVInstIShift<0b0000000, 0b101, 0b0010011, (outs GPR:$rd), (ins GPR:$rs1, shamt:$shamt), "srli\t$rd, $rs1, $shamt", [(set GPR:$rd, (srl GPR:$rs1, shamt:$shamt))]>;
  def SRAI : RVInstIShift<0b0100000, 0b101, 0b0010011, (outs GPR:$rd), (ins GPR:$rs1, shamt:$shamt), "srai\t$rd, $rs1, $shamt", [(set GPR:$rd, (sra GPR:$rs1, shamt:$shamt))]>;
}

// U-Type
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
  def LUI   : RVInstU<0b0110111, (outs GPR:$rd), (ins uimm20:$imm20), "lui\t$rd, $imm20", []>;
  def AUIPC : RVInstU<0b0010111, (outs GPR:$rd), (ins uimm20:$imm20), "auipc\t$rd, $imm20", []>;
}

// Branches
let isBranch = 1, isTerminator = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
  def BEQ  : RVInstB<0b000, 0b1100011, (outs), (ins GPR:$rs1, GPR:$rs2, brtarget:$imm13), "beq\t$rs1, $rs2, $imm13", []>;
  def BNE  : RVInstB<0b001, 0b1100011, (outs), (ins GPR:$rs1, GPR:$rs2, brtarget:$imm13), "bne\t$rs1, $rs2, $imm13", []>;
  def BLT  : RVInstB<0b100, 0b1100011, (outs), (ins GPR:$rs1, GPR:$rs2, brtarget:$imm13), "blt\t$rs1, $rs2, $imm13", []>;
  def BGE  : RVInstB<0b101, 0b1100011, (outs), (ins GPR:$rs1, GPR:$rs2, brtarget:$imm13), "bge\t$rs1, $rs2, $imm13", []>;
  def BLTU : RVInstB<0b110, 0b1100011, (outs), (ins GPR:$rs1, GPR:$rs2, brtarget:$imm13), "bltu\t$rs1, $rs2, $imm13", []>;
  def BGEU : RVInstB<0b111, 0b1100011, (outs), (ins GPR:$rs1, GPR:$rs2, brtarget:$imm13), "bgeu\t$rs1, $rs2, $imm13", []>;
}

// Jump and Link
let isCall = 1, isTerminator = 1, isBranch = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
  def JAL : RVInstJ<0b1101111, (outs GPR:$rd), (ins jmptarget:$imm21), "jal\t$rd, $imm21", []>;
}

let isBranch = 1, isTerminator = 1, isBarrier = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
  def PseudoBR : RVInstJ<0b1101111, (outs), (ins jmptarget_bb:$imm21), "j\t$imm21", [(br bb:$imm21)]> {
    let rd = 0; // X0
  }
}

let isCall = 1, isTerminator = 1, isBranch = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
  def JALR : RVInstI<0b000, 0b1100111, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "jalr\t$rd, ${imm12}(${rs1})", []>;
}

// Memory Loads
let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in {
  def LB  : RVInstI<0b000, 0b0000011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "lb\t$rd, ${imm12}(${rs1})", []>;
  def LH  : RVInstI<0b001, 0b0000011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "lh\t$rd, ${imm12}(${rs1})", []>;
  def LW  : RVInstI<0b010, 0b0000011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "lw\t$rd, ${imm12}(${rs1})", []>;
  def LBU : RVInstI<0b100, 0b0000011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "lbu\t$rd, ${imm12}(${rs1})", []>;
  def LHU : RVInstI<0b101, 0b0000011, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12), "lhu\t$rd, ${imm12}(${rs1})", []>;
}

// Memory Stores
let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in {
  def SB : RVInstS<0b000, 0b0100011, (outs), (ins GPR:$rs2, GPR:$rs1, simm12:$imm12), "sb\t$rs2, ${imm12}(${rs1})", []>;
  def SH : RVInstS<0b001, 0b0100011, (outs), (ins GPR:$rs2, GPR:$rs1, simm12:$imm12), "sh\t$rs2, ${imm12}(${rs1})", []>;
  def SW : RVInstS<0b010, 0b0100011, (outs), (ins GPR:$rs2, GPR:$rs1, simm12:$imm12), "sw\t$rs2, ${imm12}(${rs1})", []>;
}

/// Standalone Pattern Matching
/// Sometimes we want to match a DAG node to an instruction, but the instruction 
/// wasn't defined with that pattern. We can use the `Pat` class to define standalone rules.

def simm12_imm : PatLeaf<(imm), [{ return isInt<12>(N->getSExtValue()); }]>;
def : Pat<(simm12_imm:$imm), (ADDI X0, simm12_imm:$imm)>;

// Basic load/store patterns (assuming base register + offset 0 for now)
def : Pat<(store (i32 GPR:$rs2), GPR:$rs1), (SW GPR:$rs2, GPR:$rs1, 0)>;
def : Pat<(load GPR:$rs1), (LW GPR:$rs1, 0)>;
