#pragma once

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

class MCContext;
class TargetMachine;

class SDMAELFTargetObjectFile : public TargetLoweringObjectFileELF {
public:
  SDMAELFTargetObjectFile() :
    TargetLoweringObjectFileELF()
  {}

  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

  const MCExpr *getTTypeGlobalReference(const GlobalValue *GV,
                                        unsigned Encoding,
                                        const TargetMachine &TM,
                                        MachineModuleInfo *MMI,
                                        MCStreamer &Streamer) const override;
};

} // end namespace llvm

