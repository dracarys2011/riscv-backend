//===-- RV32ITargetInfo.h - RV32I Target Implementation ---------*- C++ -*-===//
//
// This file is the very first step in registering a new LLVM backend. 
// It simply exposes a global `Target` struct representing our RV32I architecture
// so that other parts of the LLVM initialization process can locate it.
//
//===----------------------------------------------------------------------===//

#pragma once
namespace llvm {
class Target;

/// getTheRV32ITarget - Returns the global Target instance for RV32I.
/// This instance acts as a central registry for all our backend's components.
Target &getTheRV32ITarget();
} // namespace llvm
