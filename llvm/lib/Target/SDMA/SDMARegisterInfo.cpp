#include "SDMARegisterInfo.h"
#include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "SDMAFrameLowering.h"
#include "SDMAMachineFunctionInfo.h"
#include "utils.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include <cstdint>

#define GET_REGINFO_TARGET_DESC
#include "SDMAGenRegisterInfo.inc"

namespace llvm {

SDMARegisterInfo::SDMARegisterInfo()
    : SDMAGenRegisterInfo(sdma::RPC, // Return Address
                          0,         // DwarfFlavor
                          0,         // EHFlavor
                          sdma::PC   // PC
      ) {}

const uint16_t *
SDMARegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

const uint32_t *
SDMARegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const {
  return CSR_RegMask;
}

BitVector SDMARegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  markSuperRegs(
      Reserved,
      sdma::GP5); // FIXME: Figure out a register we can use as temporary
  markSuperRegs(Reserved, sdma::GP6); // we use gp6 as frame pointer
  markSuperRegs(Reserved, sdma::GP7); // we use gp7 as SP
  return Reserved;
}

static void replaceFI(MachineFunction &MF, MachineBasicBlock::iterator II,
                      MachineInstr &MI, const DebugLoc &Dl,
                      unsigned FIOperandNum, int Offset, unsigned FramePtr) {
  if (Offset >= 0 && Offset < (1 << 5)) {
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
    return;
  }

  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  if (-256 < Offset && Offset <= (255 + Offset < (1 << 5))) {
    BuildMI(*MI.getParent(), II, Dl, TII.get(sdma::MOV), sdma::GP5)
        .addReg(sdma::GP7);
    unsigned Opc = sdma::ADDri;
    unsigned LoadOff = 0;

    if (Offset < 0) {
      Opc = sdma::SUBri;
      Offset = -Offset;
    }

    else if (Offset > 255) {
      LoadOff = Offset - 255;
      Offset = 255;
    }

    BuildMI(*MI.getParent(), II, Dl, TII.get(Opc), sdma::GP5)
        .addReg(sdma::GP5)
        .addImm(Offset);
    MI.getOperand(FIOperandNum).ChangeToRegister(sdma::GP5, false);
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(LoadOff);
    return;
  }

  not_implemented();
}

bool SDMARegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  DebugLoc Dl = MI.getDebugLoc();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  MachineFunction &MF = *MI.getParent()->getParent();
  const SDMAFrameLowering *TFI = getFrameLowering(MF);

  Register FrameReg;
  int Offset;
  Offset = TFI->getFrameIndexReference(MF, FrameIndex, FrameReg).getFixed();

  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  /*
  if (!Subtarget.isV9() || !Subtarget.hasHardQuad()) {
    if (MI.getOpcode() == SP::STQFri) {
      const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
      Register SrcReg = MI.getOperand(2).getReg();
      Register SrcEvenReg = getSubReg(SrcReg, SP::sub_even64);
      Register SrcOddReg = getSubReg(SrcReg, SP::sub_odd64);
      MachineInstr *StMI =
        BuildMI(*MI.getParent(), II, dl, TII.get(SP::STDFri))
        .addReg(FrameReg).addImm(0).addReg(SrcEvenReg);
      replaceFI(MF, *StMI, *StMI, dl, 0, Offset, FrameReg);
      MI.setDesc(TII.get(SP::STDFri));
      MI.getOperand(2).setReg(SrcOddReg);
      Offset += 8;
    } else if (MI.getOpcode() == SP::LDQFri) {
      const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
      Register DestReg = MI.getOperand(0).getReg();
      Register DestEvenReg = getSubReg(DestReg, SP::sub_even64);
      Register DestOddReg = getSubReg(DestReg, SP::sub_odd64);
      MachineInstr *LdMI =
        BuildMI(*MI.getParent(), II, dl, TII.get(SP::LDDFri), DestEvenReg)
        .addReg(FrameReg).addImm(0);
      replaceFI(MF, *LdMI, *LdMI, dl, 1, Offset, FrameReg);

      MI.setDesc(TII.get(SP::LDDFri));
      MI.getOperand(0).setReg(DestOddReg);
      Offset += 8;
    }
  }*/

  replaceFI(MF, II, MI, Dl, FIOperandNum, Offset, FrameReg);
  // replaceFI never removes II
  return false;
}

Register SDMARegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return getFrameLowering(MF)->hasFP(MF) ? sdma::GP6 : sdma::GP7;
}

} // namespace llvm
