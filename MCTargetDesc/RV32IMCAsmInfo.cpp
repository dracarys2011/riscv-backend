#include "RV32IMCAsmInfo.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/MC/MCTargetOptions.h"

using namespace llvm;

void RV32IMCAsmInfo::anchor() {}

RV32IMCAsmInfo::RV32IMCAsmInfo(const Triple &TargetTriple,
                               const MCTargetOptions &Options)
    : MCAsmInfoELF(Options) {
  CodePointerSize = 4;
  CalleeSaveStackSlotSize = 4;
  CommentString = "#";
  AlignmentIsInBytes = false;
  SupportsDebugInformation = true;
  Data16bitsDirective = "\t.half\t";
  Data32bitsDirective = "\t.word\t";
}
