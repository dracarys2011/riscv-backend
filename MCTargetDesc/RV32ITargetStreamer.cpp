#include "RV32ITargetStreamer.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

RV32ITargetStreamer::RV32ITargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

RV32ITargetStreamer::~RV32ITargetStreamer() = default;

RV32ITargetAsmStreamer::RV32ITargetAsmStreamer(MCStreamer &S,
                                               formatted_raw_ostream &OS)
    : RV32ITargetStreamer(S), OS(OS) {}
