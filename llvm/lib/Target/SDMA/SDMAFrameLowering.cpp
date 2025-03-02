#include "SDMAFrameLowering.h"
#include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "SDMAInstrInfo.h"
#include "SDMAMachineFunctionInfo.h"
#include "SDMARegisterInfo.h"
#include "SDMASubtarget.h"
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
                          Align(8)),
      ST(ST) {}

// emitProlog/emitEpilog - These methods insert prolog and epilog code into
/// the function.
void SDMAFrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  assert(&MF.front() == &MBB && "Shrink-wrapping not yet supported");

  MachineFrameInfo &MFI = MF.getFrameInfo();
  const SDMAInstrInfo &TII =
      *static_cast<const SDMAInstrInfo *>(ST.getInstrInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();

  // Debug location must be unknown since the first debug location is used
  // to determine the end of the prologue.
  DebugLoc DL;

  // FIXME: This appears to be overallocating.  Needs investigation.
  // Get the number of bytes to allocate from the FrameInfo.
  unsigned StackSize = MFI.getStackSize();

  // Push old FP
  // st %fp,-4[*%sp]
  buildPush(MBB, MBBI, &TII, sdma::GP6);

  // Generate new FP
  // add %sp,8,%fp
  BuildMI(MBB, MBBI, DL, TII.get(sdma::MOV), sdma::GP6).addReg(sdma::GP7);

  BuildMI(MBB, MBBI, DL, TII.get(sdma::ADDri), sdma::GP6)
      .addReg(sdma::GP6)
      .addImm(8);

  // Allocate space on the stack if needed
  // sub %sp,StackSize,%sp
  if (StackSize != 0) {
    
  BuildMI(MBB, MBBI, DL, TII.get(sdma::SUBri), sdma::GP7)
      .addReg(sdma::GP7)
      .addImm(StackSize)
      .setMIFlag(MachineInstr::FrameSetup);
  }

  if (MFI.hasVarSizedObjects())
    not_implemented();
}

void SDMAFrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  const SDMAInstrInfo &TII =
      *static_cast<const SDMAInstrInfo *>(ST.getInstrInfo());
  DebugLoc DL = MBBI->getDebugLoc();

  BuildMI(MBB, MBBI, DL, TII.get(sdma::MOV), sdma::GP7)
      .addReg(sdma::GP6);
  buildPop(MBB, MBBI, &TII, sdma::GP6);
}

MachineBasicBlock::iterator SDMAFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {

  if (!hasReservedCallFrame(MF)) {
    MachineInstr &MI = *I;
    int Size = MI.getOperand(0).getImm();
    if (MI.getOpcode() == sdma::ADJCALLSTACKDOWN)
      Size = -Size;

    if (Size)
      emitSPAdjustment(MF, MBB, I, Size);
  }
  return MBB.erase(I);
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
}

StackOffset
SDMAFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                          Register &FrameReg) const {
  const SDMASubtarget &Subtarget = MF.getSubtarget<SDMASubtarget>();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const SDMARegisterInfo *RegInfo = Subtarget.getRegisterInfo();
  const SDMAMachineFunctionInfo *FuncInfo =
      MF.getInfo<SDMAMachineFunctionInfo>();
  bool IsFixed = MFI.isFixedObjectIndex(FI);

  bool UseFP;

  if (FuncInfo->isLeafProc()) {
    UseFP = false;
  } else if (IsFixed) {
    UseFP = true;
  } else if (RegInfo->hasStackRealignment(MF)) {
    UseFP = false;
  } else {
    UseFP = true;
  }

  int64_t FrameOffset = MF.getFrameInfo().getObjectOffset(FI);

  if (UseFP) {
    FrameReg = RegInfo->getFrameRegister(MF);
    return StackOffset::getFixed(FrameOffset);
  }

  FrameReg = sdma::GP7; // %sp
  return StackOffset::getFixed(FrameOffset + MF.getFrameInfo().getStackSize());
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
                                         int NumBytes) const {
  DebugLoc Dl;
  const SDMAInstrInfo &TII =
      *static_cast<const SDMAInstrInfo *>(MF.getSubtarget().getInstrInfo());

  unsigned InstrType = sdma::ADDri;

  if (NumBytes < 0) {
    InstrType = sdma::SUBri;
  }

  if (NumBytes > 0 && NumBytes < 256) {
    BuildMI(MBB, MBBI, Dl, TII.get(InstrType))
        .addReg(sdma::GP7, RegState::Define)
        .addReg(sdma::GP7)
        .addImm(NumBytes);
    return;
  }

  not_implemented();
}

