#ifndef LLVM_LIB_TARGET_RV32I_MCTARGETDESC_RV32ITARGETSTREAMER_H
#define LLVM_LIB_TARGET_RV32I_MCTARGETDESC_RV32ITARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"

namespace llvm {
class formatted_raw_ostream;

class RV32ITargetStreamer : public MCTargetStreamer {
public:
  RV32ITargetStreamer(MCStreamer &S);
  ~RV32ITargetStreamer() override;
};

// This part is for ascii assembly output
class RV32ITargetAsmStreamer : public RV32ITargetStreamer {
  formatted_raw_ostream &OS;

public:
  RV32ITargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);
};

} // end namespace llvm
#endif
