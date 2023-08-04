#include "MCTargetDesc/SDMAMCExpr.h"
#include "SDMA.h"
#include "utils.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"

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
  const SDMAMCExpr *Expr =
      SDMAMCExpr::create((SDMAMCExpr::VariantKind)Kind, MCSym, AP.OutContext);
  return MCOperand::createExpr(Expr);
}

static MCOperand lowerOperand(const MachineInstr *MI, const MachineOperand &MO,
                              AsmPrinter &AP) {
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    if (MO.isImplicit())
      break;
    return MCOperand::createReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm());

  case MachineOperand::MO_GlobalAddress:
  case MachineOperand::MO_BlockAddress:
  case MachineOperand::MO_ExternalSymbol:
  case MachineOperand::MO_ConstantPoolIndex:
  case MachineOperand::MO_MachineBasicBlock:
    return lowerSymbolOperand(MI, MO, AP);
  case MachineOperand::MO_RegisterMask:
    break;
  case MachineOperand::MO_CImmediate:
  case MachineOperand::MO_FPImmediate:
  case MachineOperand::MO_FrameIndex:
  case MachineOperand::MO_TargetIndex:
  case MachineOperand::MO_JumpTableIndex:
  case MachineOperand::MO_RegisterLiveOut:
  case MachineOperand::MO_Metadata:
  case MachineOperand::MO_MCSymbol:
  case MachineOperand::MO_CFIIndex:
  case MachineOperand::MO_IntrinsicID:
  case MachineOperand::MO_Predicate:
  case MachineOperand::MO_ShuffleMask:
  case MachineOperand::MO_DbgInstrRef:
    not_implemented();
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
