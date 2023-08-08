#include "SDMAMCExpr.h"
#include "utils.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

#define DEBUG_TYPE "sdmamcexpr"

const SDMAMCExpr *SDMAMCExpr::create(VariantKind Kind, const MCExpr *Expr,
                                     MCContext &Ctx) {
  return new (Ctx) SDMAMCExpr(Kind, Expr);
}

void SDMAMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {

  bool CloseParen = printVariantKind(OS, Kind);

  const MCExpr *Expr = getSubExpr();
  Expr->print(OS, MAI);

  if (CloseParen)
    OS << ')';
}

bool SDMAMCExpr::printVariantKind(raw_ostream &OS, VariantKind Kind) {
  switch (Kind) {
  case VK_SDMA_None:
    return false;
  }
  llvm_unreachable("Unhandled SDMAMCExpr::VariantKind");
}

SDMAMCExpr::VariantKind SDMAMCExpr::parseVariantKind(StringRef name) {
  return StringSwitch<SDMAMCExpr::VariantKind>(name).Default(VK_SDMA_None);
}

bool SDMAMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                           const MCAsmLayout *Layout,
                                           const MCFixup *Fixup) const {
  return getSubExpr()->evaluateAsRelocatable(Res, Layout, Fixup);
}

static void fixELFSymbolsInTLSFixupsImpl(const MCExpr *Expr, MCAssembler &Asm) {
  switch (Expr->getKind()) {
  case MCExpr::Target:
    llvm_unreachable("Can't handle nested target expr!");
    break;

  case MCExpr::Constant:
    break;

  case MCExpr::Binary: {
    const MCBinaryExpr *BE = cast<MCBinaryExpr>(Expr);
    fixELFSymbolsInTLSFixupsImpl(BE->getLHS(), Asm);
    fixELFSymbolsInTLSFixupsImpl(BE->getRHS(), Asm);
    break;
  }

  case MCExpr::SymbolRef: {
    const MCSymbolRefExpr &SymRef = *cast<MCSymbolRefExpr>(Expr);
    cast<MCSymbolELF>(SymRef.getSymbol()).setType(ELF::STT_TLS);
    break;
  }

  case MCExpr::Unary:
    fixELFSymbolsInTLSFixupsImpl(cast<MCUnaryExpr>(Expr)->getSubExpr(), Asm);
    break;
  }
}

void SDMAMCExpr::fixELFSymbolsInTLSFixups(MCAssembler &Asm) const {
  switch (getKind()) {
  default:
    return;
  }
  fixELFSymbolsInTLSFixupsImpl(getSubExpr(), Asm);
}

void SDMAMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}

SDMA::Fixups SDMAMCExpr::getFixupKind(VariantKind Kind) { not_implemented(); }
