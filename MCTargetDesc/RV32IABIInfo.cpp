#include "RV32IABIInfo.h"
#include "llvm/ADT/StringSwitch.h"

using namespace llvm;

RV32IABIInfo RV32IABIInfo::computeTargetABI(const Triple &TT,
                                            StringRef ABIName) {
  ABI TargetABI = StringSwitch<ABI>(ABIName)
                      .Case("ilp32", ABI::ILP32)
                      .Default(ABI::Unknown);

  if (TargetABI == ABI::Unknown) {
    TargetABI = ABI::ILP32;
  }

  return RV32IABIInfo(TargetABI);
}
