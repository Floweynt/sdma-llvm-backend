#include "SDMASubtarget.h"
#include "SDMA.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

#define DEBUG_TYPE "sdma-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SDMAGenSubtargetInfo.inc"

SDMASubtarget::SDMASubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                             const TargetMachine &TM)
    : SDMAGenSubtargetInfo(TT, CPU, /* TuneCPU */ CPU, FS), TargetTriple(TT),
      InstrInfo(*this), TLInfo(TM, *this), FrameLowering(*this) {}

