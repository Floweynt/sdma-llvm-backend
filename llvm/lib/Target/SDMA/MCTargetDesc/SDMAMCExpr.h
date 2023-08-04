#pragma once

#include "SDMAFixupKinds.h"
#include "llvm/MC/MCExpr.h"

namespace llvm {

class StringRef;
class SDMAMCExpr : public MCTargetExpr {
public:
  enum VariantKind { VK_SDMA_None };

private:
  const VariantKind Kind;
  const MCExpr *Expr;

  explicit SDMAMCExpr(VariantKind Kind, const MCExpr *Expr)
      : Kind(Kind), Expr(Expr) {}

public:
  /// @name Construction
  /// @{

  static const SDMAMCExpr *create(VariantKind Kind, const MCExpr *Expr,
                                  MCContext &Ctx);
  /// @}
  /// @name Accessors
  /// @{

  /// getOpcode - Get the kind of this expression.
  VariantKind getKind() const { return Kind; }

  /// getSubExpr - Get the child of this expression.
  const MCExpr *getSubExpr() const { return Expr; }
  /// @}
  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }

  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override;

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  static VariantKind parseVariantKind(StringRef Name);
  static bool printVariantKind(raw_ostream &OS, VariantKind Kind);

  SDMA::Fixups getFixupKind() const { return getFixupKind(Kind); }
  static SDMA::Fixups getFixupKind(VariantKind Kind);
};

} // end namespace llvm.

