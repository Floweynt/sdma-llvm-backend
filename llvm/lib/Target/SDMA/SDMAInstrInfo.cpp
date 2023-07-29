#include "SDMAInstrInfo.h"
#include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "SDMAFrameLowering.h"
#include "SDMARegisterInfo.h"
#include "utils.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "SDMAGenInstrInfo.inc"

using namespace llvm;

void SDMAInstrInfo::anchor() { not_implemented(); }

SDMAInstrInfo::SDMAInstrInfo(SDMASubtarget &ST) : Subtarget(ST) {}

unsigned SDMAInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                            int &FrameIndex) const {
  not_implemented();
}

unsigned SDMAInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                           int &FrameIndex) const {
  not_implemented();
}

MachineBasicBlock *
SDMAInstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("unexpected opcode!");
    // we need a case for this...
    return MI.getOperand(0).getMBB();
  }
}

static bool isUncondBranchOpcode(int Opc) { return Opc == sdma::JMP; }

static bool isCondBranchOpcode(int Opc) {
  return Opc == sdma::BDF || Opc == sdma::BF || Opc == sdma::BSF ||
         Opc == sdma::BT;
}

static bool isIndirectBranchOpcode(int Opc) { return false; }

static void parseCondBranch(MachineInstr *LastInst, MachineBasicBlock *&Target,
                            SmallVectorImpl<MachineOperand> &Cond) {
  not_implemented();
}

bool SDMAInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                  MachineBasicBlock *&TBB,
                                  MachineBasicBlock *&FBB,
                                  SmallVectorImpl<MachineOperand> &Cond,
                                  bool AllowModify) const {
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end())
    return false;

  if (!isUnpredicatedTerminator(*I))
    return false;

  MachineInstr *LastInst = &*I;
  unsigned LastOpc = LastInst->getOpcode();

  // If there is only one terminator instruction, process it.
  if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
    if (isUncondBranchOpcode(LastOpc)) {
      TBB = LastInst->getOperand(0).getMBB();
      return false;
    }
    if (isCondBranchOpcode(LastOpc)) {
      // Block ends with fall-through condbranch.
      parseCondBranch(LastInst, TBB, Cond);
      return false;
    }
    return true; // Can't handle indirect branch.
  }

  // Get the instruction before it if it is a terminator.
  MachineInstr *SecondLastInst = &*I;
  unsigned SecondLastOpc = SecondLastInst->getOpcode();

  // If AllowModify is true and the block ends with two or more unconditional
  // branches, delete all but the first unconditional branch.
  if (AllowModify && isUncondBranchOpcode(LastOpc)) {
    while (isUncondBranchOpcode(SecondLastOpc)) {
      LastInst->eraseFromParent();
      LastInst = SecondLastInst;
      LastOpc = LastInst->getOpcode();
      if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
        // Return now the only terminator is an unconditional branch.
        TBB = LastInst->getOperand(0).getMBB();
        return false;
      }

      SecondLastInst = &*I;
      SecondLastOpc = SecondLastInst->getOpcode();
    }
  }

  // If there are three terminators, we don't know what sort of block this is.
  if (SecondLastInst && I != MBB.begin() && isUnpredicatedTerminator(*--I))
    return true;

  // If the block ends with a B and a Bcc, handle it.
  if (isCondBranchOpcode(SecondLastOpc) && isUncondBranchOpcode(LastOpc)) {
    parseCondBranch(SecondLastInst, TBB, Cond);
    FBB = LastInst->getOperand(0).getMBB();
    return false;
  }

  // If the block ends with two unconditional branches, handle it.  The second
  // one is not executed.
  if (isUncondBranchOpcode(SecondLastOpc) && isUncondBranchOpcode(LastOpc)) {
    TBB = SecondLastInst->getOperand(0).getMBB();
    return false;
  }

  // ...likewise if it ends with an indirect branch followed by an unconditional
  // branch.
  if (isIndirectBranchOpcode(SecondLastOpc) && isUncondBranchOpcode(LastOpc)) {
    I = LastInst;
    if (AllowModify)
      I->eraseFromParent();
    return true;
  }

  // Otherwise, can't handle this.
  return true;
}

unsigned SDMAInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                     int *BytesRemoved) const {
  not_implemented();
}

unsigned SDMAInstrInfo::insertBranch(
    MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB,
    ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {
  not_implemented();
}

bool SDMAInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  not_implemented();
}

/// Determine if the branch target is in range.
bool SDMAInstrInfo::isBranchOffsetInRange(unsigned BranchOpc,
                                          int64_t Offset) const {
  not_implemented();
}

void SDMAInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I,
                                const DebugLoc &DL, MCRegister DestReg,
                                MCRegister SrcReg, bool KillSrc) const {

  if (sdma::GPRegsRegClass.contains(DestReg, SrcReg))
    BuildMI(MBB, I, DL, get(sdma::MOV), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
}

void SDMAInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register SrcReg,
    bool IsKill, int FrameIndex, const TargetRegisterClass *RC,
    const TargetRegisterInfo *TRI, Register VReg) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  MachineFrameInfo &MFI = MF->getFrameInfo();

  if (sdma::GPRegsRegClass.hasSubClassEq(RC)) {
    MachineMemOperand *MMO = MF->getMachineMemOperand(
        MachinePointerInfo::getFixedStack(*MF, FrameIndex),
        MachineMemOperand::MOStore, MFI.getObjectSize(FrameIndex),
        MFI.getObjectAlign(FrameIndex));

    BuildMI(MBB, I, DL, get(sdma::PseudoPUSH))
        .addReg(SrcReg, getKillRegState(IsKill))
        //       .addFrameIndex(FrameIndex)
        //.addImm(0)
        ; // .addMemOperand(MMO);
  } else
    llvm_unreachable("Can't store this register to stack slot");
}

void SDMAInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator I,
                                         Register DestReg, int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI,
                                         Register VReg) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  MachineFrameInfo &MFI = MF->getFrameInfo();

  unsigned Opcode;
  if (sdma::GPRegsRegClass.hasSubClassEq(RC)) {
    MachineMemOperand *MMO = MF->getMachineMemOperand(
        MachinePointerInfo::getFixedStack(*MF, FrameIndex),
        MachineMemOperand::MOLoad, MFI.getObjectSize(FrameIndex),
        MFI.getObjectAlign(FrameIndex));

    BuildMI(MBB, I, DL, get(sdma::PseudoPOP), DestReg);
  } else
    llvm_unreachable("Can't load this register from stack slot");
}

Register SDMAInstrInfo::getGlobalBaseReg(MachineFunction *MF) const {
  not_implemented();
}

unsigned SDMAInstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  not_implemented();
}

bool SDMAInstrInfo::expandPostRAPseudo(MachineInstr &MI) const { return false; }
