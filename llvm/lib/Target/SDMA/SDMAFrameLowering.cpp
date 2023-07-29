#include "SDMAFrameLowering.h"
#include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "SDMAMachineFunctionInfo.h"
#include "utils.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include <iostream>
#include <system_error>
using namespace llvm;

SDMAFrameLowering::SDMAFrameLowering(const SDMASubtarget &ST)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(8), 0,
                          Align(8)) {}

// emitProlog/emitEpilog - These methods insert prolog and epilog code into
/// the function.
void SDMAFrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.begin();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const Function &Fn = MF.getFunction();
  MachineModuleInfo &MMI = MF.getMMI();
  SDMAMachineFunctionInfo *SDMAFI = MF.getInfo<SDMAMachineFunctionInfo>();

  std::cerr << "TODO: " << __FILE__ << ":" << __LINE__ << '\n';
}

void SDMAFrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {}

MachineBasicBlock::iterator SDMAFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  std::cerr << "TODO: " << __FILE__ << ":" << __LINE__ << '\n';
  not_implemented();
}

bool SDMAFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  not_implemented();
}

bool SDMAFrameLowering::hasFP(const MachineFunction &MF) const {
  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();

  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         RegInfo->hasStackRealignment(MF) || MFI.hasVarSizedObjects() ||
         MFI.isFrameAddressTaken();
}

void SDMAFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                             BitVector &SavedRegs,
                                             RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  if (isLeafProc(MF)) {
    SDMAMachineFunctionInfo *MFI = MF.getInfo<SDMAMachineFunctionInfo>();
    MFI->setLeafProc(true);

    remapRegsForLeafProc(MF);
  }
}

StackOffset
SDMAFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                          Register &FrameReg) const {
  not_implemented();
}

void SDMAFrameLowering::remapRegsForLeafProc(MachineFunction &MF) const {
  std::cerr << "TODO: optimize this\n";
}

bool SDMAFrameLowering::isLeafProc(MachineFunction &MF) const {
  MachineRegisterInfo &MRI = MF.getRegInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  return !(MFI.hasCalls() // has calls
                          // || MRI.isPhysRegUsed(SP::L0)    // Too many
                          // registers needed
           || MRI.isPhysRegUsed(sdma::GP7)     // %sp is used
           || hasFP(MF) || MF.hasInlineAsm()); // need %fp
}

void SDMAFrameLowering::emitSPAdjustment(MachineFunction &MF,
                                         MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MBBI,
                                         int NumBytes, unsigned ADDrr,
                                         unsigned ADDri) const {
  not_implemented();
}
