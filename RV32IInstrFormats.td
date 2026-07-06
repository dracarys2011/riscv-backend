//===-- RV32IInstrFormats.td - RV32I Instruction Formats ---*- tablegen -*-===//
//
// The RISC-V ISA is famous for its clean and orthogonal instruction formats.
// There are 6 basic formats (R, I, S, B, U, J). This file models exactly how 
// bits are sliced up for each format.
//
// By defining these formats as TableGen classes, we can simply instantiate 
// an instruction like `ADD` by providing its unique opcodes and funct bits,
// and TableGen will automatically figure out how to encode it into a 32-bit integer!
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Base Instruction Format
//===----------------------------------------------------------------------===//

/// RV32IInst: The grand-daddy of all RV32I instructions.
/// It defines the standard 32-bit field `Inst`. All RISC-V instructions have 
/// a 7-bit opcode in the lowest bits (`Inst{6-0}`).
class RV32IInst<bits<7> op, dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction {
  field bits<32> Inst;

  let Namespace = "RV32I";

  bits<7> Opcode = op;
  let Inst{6-0} = Opcode;

  let OutOperandList = outs;
  let InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
  let Size = 4;
}

//===----------------------------------------------------------------------===//
// R-Type Instruction Format
//===----------------------------------------------------------------------===//
/// R-Type (Register Type) instructions are used for register-to-register 
/// operations like `add`, `sub`, `and`.
/// They have 3 register operands: one destination (`rd`) and two sources (`rs1`, `rs2`).
/// The opcode is shared, but operations are distinguished by `funct3` and `funct7`.

class RVInstR<bits<7> funct7, bits<3> funct3, bits<7> op,
              dag outs, dag ins, string asmstr, list<dag> pattern>
    : RV32IInst<op, outs, ins, asmstr, pattern> {
  bits<5> rs2;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-25} = funct7;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7}  = rd;
}

//===----------------------------------------------------------------------===//
// I-Type Instruction Format
//===----------------------------------------------------------------------===//
/// I-Type (Immediate Type) instructions are used for operations with a 
/// 12-bit immediate, like `addi` or loads (`lw`).
/// They have 2 register operands (`rd`, `rs1`) and a 12-bit signed immediate (`imm12`).

class RVInstI<bits<3> funct3, bits<7> op,
              dag outs, dag ins, string asmstr, list<dag> pattern>
    : RV32IInst<op, outs, ins, asmstr, pattern> {
  bits<12> imm12;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-20} = imm12;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7}  = rd;
}

// I-Type Shift Formats (Special casing the upper 7 bits)
class RVInstIShift<bits<7> imm11_5, bits<3> funct3, bits<7> op,
                   dag outs, dag ins, string asmstr, list<dag> pattern>
    : RV32IInst<op, outs, ins, asmstr, pattern> {
  bits<5> shamt;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-25} = imm11_5;
  let Inst{24-20} = shamt;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7}  = rd;
}
//===----------------------------------------------------------------------===//
// S-Type Instruction Format
//===----------------------------------------------------------------------===//
/// S-Type (Store Type) instructions are used for memory stores (e.g., `sw`).
/// They take two source registers (`rs1` base address, `rs2` value to store)
/// and a 12-bit immediate offset.
/// Notably, the 12-bit immediate is split into two disjoint fields in the encoding!

class RVInstS<bits<3> funct3, bits<7> op,
              dag outs, dag ins, string asmstr, list<dag> pattern>
    : RV32IInst<op, outs, ins, asmstr, pattern> {
  bits<12> imm12;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31-25} = imm12{11-5};
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7}  = imm12{4-0};
}

//===----------------------------------------------------------------------===//
// B-Type Instruction Format
//===----------------------------------------------------------------------===//
// B-type instructions are like S-type but the immediate is shuffled.
// The immediate represents a 13-bit value (in multiples of 2).

class RVInstB<bits<3> funct3, bits<7> op,
              dag outs, dag ins, string asmstr, list<dag> pattern>
    : RV32IInst<op, outs, ins, asmstr, pattern> {
  bits<13> imm13;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31}    = imm13{12};
  let Inst{30-25} = imm13{10-5};
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-8}  = imm13{4-1};
  let Inst{7}     = imm13{11};
}

//===----------------------------------------------------------------------===//
// U-Type Instruction Format
//===----------------------------------------------------------------------===//
// U-type instructions have a single destination register and a 20-bit immediate.

class RVInstU<bits<7> op,
              dag outs, dag ins, string asmstr, list<dag> pattern>
    : RV32IInst<op, outs, ins, asmstr, pattern> {
  bits<20> imm20;
  bits<5> rd;

  let Inst{31-12} = imm20;
  let Inst{11-7}  = rd;
}

//===----------------------------------------------------------------------===//
// J-Type Instruction Format
//===----------------------------------------------------------------------===//
// J-type instructions have a single destination register and a 21-bit immediate,
// shuffled to optimize decode.

class RVInstJ<bits<7> op,
              dag outs, dag ins, string asmstr, list<dag> pattern>
    : RV32IInst<op, outs, ins, asmstr, pattern> {
  bits<21> imm21;
  bits<5> rd;

  let Inst{31}    = imm21{20};
  let Inst{30-21} = imm21{10-1};
  let Inst{20}    = imm21{11};
  let Inst{19-12} = imm21{19-12};
  let Inst{11-7}  = rd;
}

//===----------------------------------------------------------------------===//
// Pseudo Instructions Format
//===----------------------------------------------------------------------===//

class RV32IPseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction {
  let Namespace = "RV32I";
  let OutOperandList = outs;
  let InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}


