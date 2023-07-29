#include "MCTargetDesc/SDMAMCExpr.h"
#include "SDMA.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"

using namespace llvm;

static MCOperand lowerSymbolOperand(const MachineInstr *MI,
                                    const MachineOperand &MO, AsmPrinter &AP) {
  auto Kind = MO.getTargetFlags();
  const MCSymbol *Symbol = nullptr;

  switch (MO.getType()) {
  default:
    llvm_unreachable("Unknown type in LowerSymbolOperand");
  case MachineOperand::MO_MachineBasicBlock:
    Symbol = MO.getMBB()->getSymbol();
    break;

  case MachineOperand::MO_GlobalAddress:
    Symbol = AP.getSymbol(MO.getGlobal());
    break;

  case MachineOperand::MO_BlockAddress:
    Symbol = AP.GetBlockAddressSymbol(MO.getBlockAddress());
    break;

  case MachineOperand::MO_ExternalSymbol:
    Symbol = AP.GetExternalSymbolSymbol(MO.getSymbolName());
    break;

  case MachineOperand::MO_ConstantPoolIndex:
    Symbol = AP.GetCPISymbol(MO.getIndex());
    break;
  }

  const MCSymbolRefExpr *MCSym = MCSymbolRefExpr::create(Symbol, AP.OutContext);
  const SDMAMCExpr *expr =
      SDMAMCExpr::create((SDMAMCExpr::VariantKind)Kind, MCSym, AP.OutContext);
  return MCOperand::createExpr(expr);
}

static MCOperand lowerOperand(const MachineInstr *MI, const MachineOperand &MO,
                              AsmPrinter &AP) {
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    if (MO.isImplicit())
      break;
    return MCOperand::createReg(MO.getReg());
  case MachineOperand::MO_GlobalAddress:
    return lowerSymbolOperand(MI, MO, AP);
  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm());
  case MachineOperand::MO_BlockAddress:
    return lowerSymbolOperand(MI, MO, AP);
  }
  return MCOperand();
}

void llvm::LowerSDMAMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                         AsmPrinter &AP) {

  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp = lowerOperand(MI, MO, AP);

    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}
