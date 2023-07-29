#pragma once

// #include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class AsmPrinter;
class FunctionPass;
class MCInst;
class MachineInstr;
class PassRegistry;
class SDMATargetMachine;

FunctionPass *createSDMAISelDag(SDMATargetMachine &TM);
FunctionPass *createSDMADelaySlotFillerPass();
FunctionPass * createSDMAExpandPseudoPass();

void LowerSDMAMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                   AsmPrinter &AP);
void initializeSDMADAGToDAGISelPass(PassRegistry &);
void initializeSDMAExpandPseudoPass(PassRegistry &);
} // namespace llvm
