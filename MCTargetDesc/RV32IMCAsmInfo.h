#ifndef LLVM_LIB_TARGET_RV32I_MCTARGETDESC_RV32IMCASMINFO_H
#define LLVM_LIB_TARGET_RV32I_MCTARGETDESC_RV32IMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/MC/MCTargetOptions.h"

namespace llvm {
class Triple;

class RV32IMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit RV32IMCAsmInfo(const Triple &TargetTriple,
                          const MCTargetOptions &Options);
};
} // namespace llvm

#endif
