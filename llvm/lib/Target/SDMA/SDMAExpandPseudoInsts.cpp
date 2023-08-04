//===-- SDMAExpandPseudoInsts.cpp - Expand pseudo instructions
//-------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a pass that expands pseudo instructions into target
// instructions. This pass should be run after register allocation but before
// the post-regalloc scheduling pass.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "SDMA.h"
#include "SDMAInstrInfo.h"
#include "SDMATargetMachine.h"

#include "utils.h"
#include "llvm/ADT/bit.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include <cstdint>

using namespace llvm;

#define SDMA_EXPAND_PSEUDO_NAME "SDMA pseudo instruction expansion pass"

namespace {

/// Expands "placeholder" instructions marked as pseudo into
/// actual SDMA instructions.
class SDMAExpandPseudo : public MachineFunctionPass {
public:
  static char ID;

  SDMAExpandPseudo() : MachineFunctionPass(ID) {
    initializeSDMAExpandPseudoPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return SDMA_EXPAND_PSEUDO_NAME; }

private:
  typedef MachineBasicBlock Block;
  typedef Block::iterator BlockIt;

  const SDMARegisterInfo *TRI;
  const TargetInstrInfo *TII;

  bool expandMBB(Block &MBB);
  bool expandMI(Block &MBB, BlockIt MBBI);
  template <unsigned OP> bool expand(Block &MBB, BlockIt MBBI);

  MachineInstrBuilder buildMI(Block &MBB, BlockIt MBBI, unsigned Opcode) {
    return BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(Opcode));
  }

  MachineInstrBuilder buildMI(Block &MBB, BlockIt MBBI, unsigned Opcode,
                              unsigned DstReg) {
    return BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(Opcode), DstReg);
  }

  MachineRegisterInfo &getRegInfo(Block &MBB) {
    return MBB.getParent()->getRegInfo();
  }

  bool expandArith(unsigned OpLo, unsigned OpHi, Block &MBB, BlockIt MBBI);
};

char SDMAExpandPseudo::ID = 0;

bool SDMAExpandPseudo::expandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  BlockIt MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    BlockIt NMBBI = std::next(MBBI);
    Modified |= expandMI(MBB, MBBI);
    MBBI = NMBBI;
  }

  return Modified;
}

bool SDMAExpandPseudo::runOnMachineFunction(MachineFunction &MF) {
  bool Modified = false;

  const SDMASubtarget &STI = MF.getSubtarget<SDMASubtarget>();
  TRI = STI.getRegisterInfo();
  TII = STI.getInstrInfo();

  // We need to track liveness in order to use register scavenging.
  MF.getProperties().set(MachineFunctionProperties::Property::TracksLiveness);

  for (Block &MBB : MF) {
    bool ContinueExpanding = true;
    unsigned ExpandCount = 0;

    // Continue expanding the block until all pseudos are expanded.
    do {
      assert(ExpandCount < 10 && "pseudo expand limit reached");

      bool BlockModified = expandMBB(MBB);
      Modified |= BlockModified;
      ExpandCount++;

      ContinueExpanding = BlockModified;
    } while (ContinueExpanding);
  }

  return Modified;
}

template <>
bool SDMAExpandPseudo::expand<sdma::PseudoLoadLargeU>(Block &MBB,
                                                      BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  Register TargetReg = MI.getOperand(0).getReg();
  uint64_t UImm = bit_cast<uint64_t>(MI.getOperand(1).getImm());

  loadArbitraryConstant(MBB, MBBI, TII, TargetReg, UImm);
  MI.eraseFromParent();

  return true;
}

template <>
bool SDMAExpandPseudo::expand<sdma::PseudoADDri>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  Register TargetReg = MI.getOperand(1).getReg();
  int64_t Imm = MI.getOperand(2).getImm();

  if (Imm < 0) {
    buildMI(MBB, MBBI, sdma::SUBri, TargetReg).addReg(TargetReg).addImm(-Imm);

    MI.eraseFromParent();
    return true;
  }

  llvm_unreachable("PseudoADDri is for negative values");
}

template <>
bool SDMAExpandPseudo::expand<sdma::PseudoASR>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  Register TargetReg = MI.getOperand(1).getReg();
  int64_t Amount = MI.getOperand(2).getImm();

  while (Amount--) {
    buildMI(MBB, MBBI, sdma::ASR1, TargetReg).addReg(TargetReg);
  }

  MI.eraseFromParent();
  return true;
}

template <>
bool SDMAExpandPseudo::expand<sdma::PseudoLSL>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  Register TargetReg = MI.getOperand(1).getReg();
  int64_t Amount = MI.getOperand(2).getImm();

  while (Amount--) {
    buildMI(MBB, MBBI, sdma::LSL1, TargetReg).addReg(TargetReg);
  }

  MI.eraseFromParent();
  return true;
}

template <>
bool SDMAExpandPseudo::expand<sdma::PseudoLSR>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  Register TargetReg = MI.getOperand(1).getReg();
  int64_t Amount = MI.getOperand(2).getImm();

  while (Amount--) {
    buildMI(MBB, MBBI, sdma::LSR1, TargetReg).addReg(TargetReg);
  }

  MI.eraseFromParent();
  return true;
}

#define EXPAND(Op)                                                             \
  case Op:                                                                     \
    return expand<Op>(MBB, MI)

bool SDMAExpandPseudo::expandMI(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  int Opcode = MBBI->getOpcode();

  switch (Opcode) {
  case sdma::PseudoLoadLargeS:
    EXPAND(sdma::PseudoLoadLargeU);
    EXPAND(sdma::PseudoADDri);
    EXPAND(sdma::PseudoASR);
    EXPAND(sdma::PseudoLSR);
    EXPAND(sdma::PseudoLSL);
  }
  return false;
}

#undef EXPAND

} // end of anonymous namespace

INITIALIZE_PASS(SDMAExpandPseudo, "sdma-expand-pseudo", SDMA_EXPAND_PSEUDO_NAME,
                false, false)

namespace llvm {
FunctionPass *createSDMAExpandPseudoPass() { return new SDMAExpandPseudo(); }
} // end of namespace llvm
