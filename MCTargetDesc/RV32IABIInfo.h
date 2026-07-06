#ifndef LLVM_LIB_TARGET_RV32I_MCTARGETDESC_RV32IABIINFO_H
#define LLVM_LIB_TARGET_RV32I_MCTARGETDESC_RV32IABIINFO_H

#include "llvm/TargetParser/Triple.h"

namespace llvm {
class StringRef;

class RV32IABIInfo {
public:
  enum class ABI { Unknown, ILP32 };

private:
  ABI ThisABI;

public:
  RV32IABIInfo(ABI ThisABI) : ThisABI(ThisABI) {}

  static RV32IABIInfo computeTargetABI(const Triple &TT, StringRef ABIName);

  ABI getABI() const { return ThisABI; }
};

} // namespace llvm

#endif
