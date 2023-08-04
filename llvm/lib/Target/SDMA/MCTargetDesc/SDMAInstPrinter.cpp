#include "SDMAInstPrinter.h"
#include "SDMA.h"
#include "utils.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define GET_INSTRUCTION_NAME
#define PRINT_ALIAS_INSTR
#include "SDMAGenAsmWriter.inc"

void SDMAInstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) const {
  OS << '%' << StringRef(getRegisterName(Reg)).lower();
}

void SDMAInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &O) {
  if (!printAliasInstr(MI, Address, O))
    printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void SDMAInstPrinter::printOperand(const MCInst *MI, int OpNum,
                                   raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNum);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    switch (MI->getOpcode()) {
    default:
      O << (int)MO.getImm();
      return;
    }
  }

  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MO.getExpr()->print(O, &MAI);
}

void SDMAInstPrinter::printBranchTarget(const MCInst *MI, int OpNum,
                                        raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNum);
  assert(Op.isExpr() && "Unknown pcrel immediate operand");
  O << *Op.getExpr();
}

inline static constexpr const char *CondNames[] = {"eq", "lt", "hs"};

void SDMAInstPrinter::printCondType(const MCInst *MI, int OpNum,
                                    raw_ostream &O) {
  O << CondNames[MI->getOperand(OpNum).getImm()];
}

void SDMAInstPrinter::printMemOperand(const MCInst *MI, int OpNum,
                                      raw_ostream &OS) {
  printOperand(MI, OpNum, OS);
  OS << "+";
  printOperand(MI, OpNum + 1, OS);
}
