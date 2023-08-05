#pragma once

#include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "SDMAGenRegisterInfo.inc"

namespace llvm {
class SDMARegisterInfo : public SDMAGenRegisterInfo {
public:
  SDMARegisterInfo();

  const uint16_t *
  getCalleeSavedRegs(const MachineFunction *MF = nullptr) const override;

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;

  inline const TargetRegisterClass *
  getPointerRegClass(const MachineFunction &MF, unsigned Kind) const override {
    return &sdma::GPRegsRegClass;
  }
};
} // namespace llvm
