#pragma once
#include "SDMAFrameLowering.h"
#include "SDMAISelLowering.h"
#include "SDMAInstrInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

#define GET_SUBTARGETINFO_HEADER
#include "SDMAGenSubtargetInfo.inc"

namespace llvm {
class SDMASubtarget final : public SDMAGenSubtargetInfo {
  Triple TargetTriple;
  SDMAInstrInfo InstrInfo;
  SDMATargetLowering TLInfo;
  SDMAFrameLowering FrameLowering;

public:
  SDMASubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                const TargetMachine &TM);

  const SDMAInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const SDMAFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const SDMARegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }

  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  const SDMATargetLowering *getTargetLowering() const override { return &TLInfo; }
};
} // namespace llvm
