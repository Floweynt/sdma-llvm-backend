#pragma once

// #include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace SDMACC {
  // SDMA specific condition code.
  enum CondCodes {
    COND_EQ  = 0,
    COND_NEQ = 4,
    COND_LT = 1, 
    COND_HS = 2,
    COND_INVALID = -1
  };
} // namespace SDMACC

namespace llvm {
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
} // namespace llvm
