#include "MCTargetDesc/SDMAFixupKinds.h"
#include "MCTargetDesc/SDMAMCExpr.h"
#include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "utils.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class SDMAELFObjectWriter : public MCELFObjectTargetWriter {
public:
  SDMAELFObjectWriter(bool Is64Bit, uint8_t OSABI)
      : MCELFObjectTargetWriter(Is64Bit, OSABI,
                                Is64Bit ? ELF::EM_SPARCV9 : ELF::EM_SPARC,
                                /*HasRelocationAddend*/ true) {}

  ~SDMAELFObjectWriter() override = default;

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;

  bool needsRelocateWithSymbol(const MCSymbol &Sym,
                               unsigned Type) const override;
};
} // namespace

unsigned SDMAELFObjectWriter::getRelocType(MCContext &Ctx,
                                           const MCValue &Target,
                                           const MCFixup &Fixup,
                                           bool IsPCRel) const {
  MCFixupKind Kind = Fixup.getKind();
  if (Kind >= FirstLiteralRelocationKind)
    return Kind - FirstLiteralRelocationKind;

  if (const SDMAMCExpr *SExpr = dyn_cast<SDMAMCExpr>(Fixup.getValue())) {
    /*if (SExpr->getKind() == SDMAMCExpr::VK_SDMA_R_DISP32)
      return ELF::R_SPARC_DISP32;*/

    not_implemented();
  }

  if (IsPCRel) {
    switch (Fixup.getTargetKind()) {
    default:
      llvm_unreachable("Unimplemented fixup -> relocation");
    case FK_Data_1:
      return ELF::R_SPARC_DISP8;
    case FK_Data_2:
      return ELF::R_SPARC_DISP16;
    case FK_Data_4:
      return ELF::R_SPARC_DISP32;
    case FK_Data_8:
      return ELF::R_SPARC_DISP64;
      not_implemented();
    }
  }

  switch (Fixup.getTargetKind()) {
  default:
    llvm_unreachable("Unimplemented fixup -> relocation");
  case FK_NONE:
    return ELF::R_SPARC_NONE;
  case FK_Data_1:
    return ELF::R_SPARC_8;
  case FK_Data_2:
    return ((Fixup.getOffset() % 2) ? ELF::R_SPARC_UA16 : ELF::R_SPARC_16);
  case FK_Data_4:
    return ((Fixup.getOffset() % 4) ? ELF::R_SPARC_UA32 : ELF::R_SPARC_32);
  case FK_Data_8:
    return ((Fixup.getOffset() % 8) ? ELF::R_SPARC_UA64 : ELF::R_SPARC_64);
  }

  return ELF::R_SPARC_NONE;
}

bool SDMAELFObjectWriter::needsRelocateWithSymbol(const MCSymbol &Sym,
                                                  unsigned Type) const {
  switch (Type) {
  default:
    return false;

  // All relocations that use a GOT need a symbol, not an offset, as
  // the offset of the symbol within the section is irrelevant to
  // where the GOT entry is. Don't need to list all the TLS entries,
  // as they're all marked as requiring a symbol anyways.
  case ELF::R_SPARC_GOT10:
  case ELF::R_SPARC_GOT13:
  case ELF::R_SPARC_GOT22:
  case ELF::R_SPARC_GOTDATA_HIX22:
  case ELF::R_SPARC_GOTDATA_LOX10:
  case ELF::R_SPARC_GOTDATA_OP_HIX22:
  case ELF::R_SPARC_GOTDATA_OP_LOX10:
    return true;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createSDMAELFObjectWriter(bool Is64Bit, uint8_t OSABI) {
  return std::make_unique<SDMAELFObjectWriter>(Is64Bit, OSABI);
}
