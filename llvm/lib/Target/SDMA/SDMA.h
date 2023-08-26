#pragma once

#include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "utils.h"
#include "llvm/ADT/bit.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
namespace SDMACC {
// SDMA specific condition code.
enum CondCodes {
  COND_EQ = 1,
  COND_LT = 2,
  COND_HS = 3,
  COND_INVALID = -1,
};
} // namespace SDMACC

class AsmPrinter;
class FunctionPass;
class MCInst;
class MachineInstr;
class PassRegistry;
class SDMATargetMachine;

FunctionPass *createSDMAISelDag(SDMATargetMachine &TM);
FunctionPass *createSDMADelaySlotFillerPass();
FunctionPass *createSDMAExpandPseudoPass();

void LowerSDMAMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                   AsmPrinter &AP);
void initializeSDMADAGToDAGISelPass(PassRegistry &);
void initializeSDMAExpandPseudoPass(PassRegistry &);

inline void loadArbitraryConstant(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MBBI,
                                  const TargetInstrInfo *TII, unsigned Reg,
                                  uint32_t Val) {

  if (0 <= Val && Val < 256) {
    BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(sdma::LDI), Reg)
        .addImm(Val);
    return;
  }

  int32_t SignedVal = bit_cast<int32_t>(Val);

  if (-256 < SignedVal && SignedVal < 0) {
    BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(sdma::LDI), Reg).addImm(0);
    BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(sdma::SUBri), Reg)
        .addReg(Reg)
        .addImm(0);
  }

  if (Val <= UINT16_MAX) {
    // we can use some tricks
    BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(sdma::LDI), Reg)
        .addImm((Val & 0xff00) >> 8);

    BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(sdma::REVBLO), Reg)
        .addReg(Reg);

    BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(sdma::ORri), Reg)
        .addReg(Reg)
        .addImm(Val & 0xff);
  }

  not_implemented();
}

inline void buildPush(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                      const TargetInstrInfo *TII, unsigned Reg) {
  BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(sdma::SUBri), sdma::GP7)
      .addReg(sdma::GP7)
      .addImm(4);

  BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(sdma::ST))
      .addReg(Reg)
      .addImm(0)
      .addReg(Reg);
}

inline void buildPop(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                     const TargetInstrInfo *TII, unsigned Reg) {
  BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(sdma::LD), Reg)
      .addReg(Reg)
      .addImm(0);

  BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(sdma::ADDri), sdma::GP7)
      .addReg(sdma::GP7)
      .addImm(4);
}

} // namespace llvm
