#include "SDMAMCTargetDesc.h"
#include "MCTargetDesc/SDMAInstPrinter.h"
#include "SDMATargetStreamer.h"
#include "TargetInfo/SDMATargetInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include <iostream>

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "SDMAGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "SDMAGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "SDMAGenRegisterInfo.inc"

#include "llvm/Support/Compiler.h"

static MCTargetStreamer *
createObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new SDMATargetELFStreamer(S);
}

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint,
                                                 bool IsVerboseAsm) {
  return new SDMATargetAsmStreamer(S, OS);
}

static MCTargetStreamer *createNullTargetStreamer(MCStreamer &S) {
  return new SDMATargetStreamer(S);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSDMATargetMC() {
  RegisterMCAsmInfoFn X(getTheSDMATarget(),
                        [](const MCRegisterInfo &MRI, const Triple &TT,
                           const MCTargetOptions &Options) {
                          MCAsmInfo *MAI = new MCAsmInfo();
                          return MAI;
                        });

  TargetRegistry::RegisterMCInstrInfo(getTheSDMATarget(), []() {
    MCInstrInfo *X = new MCInstrInfo();
    InitSDMAMCInstrInfo(X);
    return X;
  });

  TargetRegistry::RegisterMCRegInfo(getTheSDMATarget(), [](const Triple &TT) {
    MCRegisterInfo *X = new MCRegisterInfo();
    InitSDMAMCRegisterInfo(X, 0);
    return X;
  });

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(
      getTheSDMATarget(), [](const Triple &TT, StringRef CPU, StringRef FS) {
        return createSDMAMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
      });

  // Register the MC Code Emitter.
  TargetRegistry::RegisterMCCodeEmitter(getTheSDMATarget(),
                                        createSDMAMCCodeEmitter);

  // Register the asm backend.
  TargetRegistry::RegisterMCAsmBackend(getTheSDMATarget(),
                                       createSDMAAsmBackend);

  // Register the object target streamer.
  TargetRegistry::RegisterObjectTargetStreamer(getTheSDMATarget(),
                                               createObjectTargetStreamer);

  // Register the asm streamer.
  TargetRegistry::RegisterAsmTargetStreamer(getTheSDMATarget(),
                                            createTargetAsmStreamer);

  // Register the null streamer.
  TargetRegistry::RegisterNullTargetStreamer(getTheSDMATarget(),
                                             createNullTargetStreamer);
  // Register the MCInstPrinter
  TargetRegistry::RegisterMCInstPrinter(
      getTheSDMATarget(),
      [](const Triple &T, unsigned SyntaxVariant, const MCAsmInfo &MAI,
         const MCInstrInfo &MII, const MCRegisterInfo &MRI) {
        return (MCInstPrinter *)new SDMAInstPrinter(MAI, MII, MRI);
      });
}
