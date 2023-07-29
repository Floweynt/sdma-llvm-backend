#include "SDMATargetObjectFile.h"
// #include "MCTargetDesc/SDMAMCExpr.h"
#include "utils.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/CodeGen/MachineModuleInfoImpls.h"
#include "llvm/CodeGen/TargetLowering.h"

using namespace llvm;

void SDMAELFTargetObjectFile::Initialize(MCContext &Ctx,
                                         const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
  //  InitializeELF(TM.Options.UseInitArray);
}

const MCExpr *SDMAELFTargetObjectFile::getTTypeGlobalReference(
    const GlobalValue *GV, unsigned Encoding, const TargetMachine &TM,
    MachineModuleInfo *MMI, MCStreamer &Streamer) const {
  not_implemented();
  ;
}
