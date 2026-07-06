//===-- RV32ICallingConv.td - Calling Conventions for RV32I --*- tablegen -*-===//
//
// How do functions pass arguments to each other? How do they return values?
// This file defines the Calling Convention (ABI) for RISC-V.
// We specify which registers are used for arguments (`a0-a7`), which are 
// used for return values (`a0-a1`), and which registers must be preserved 
// across function calls (Callee-Saved Registers).
//
//===----------------------------------------------------------------------===//

/// RetCC_RV32I: Return Value Calling Convention.
/// Specifies that 32-bit integers are returned in registers X10 (a0) and X11 (a1).
def RetCC_RV32I : CallingConv<[
  // i32 are returned in registers X10, X11 (a0, a1)
  CCIfType<[i32], CCAssignToReg<[X10, X11]>>
]>;

/// CC_RV32I: The main C Calling Convention for arguments.
def CC_RV32I : CallingConv<[
  // Promote i8/i16 arguments to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // Pass integer values in registers X10-X17 (a0-a7).
  CCIfType<[i32], CCAssignToReg<[X10, X11, X12, X13, X14, X15, X16, X17]>>,

  // Integer values that get beyond the registers are passed on the stack.
  // The stack is 4-byte aligned and 4-byte slots.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

/// CSR_RV32I: Callee-Saved Registers.
/// If a function uses these registers, it MUST save them to the stack in its 
/// prologue and restore them in its epilogue.
/// X1 (ra/return address), X8 (fp/frame pointer), X9 (s1), X18-X27 (s2-s11).
def CSR_RV32I : CalleeSavedRegs<(add X1, X8, X9,
                                     (sequence "X%u", 18, 27))>;
