#include "MCTargetDesc/SDMAFixupKinds.h"
#include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "utils.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/Error.h"

using namespace llvm;

static unsigned adjustFixupValue(unsigned Kind, uint64_t Value) {
  switch (Kind) {
  default:
    not_implemented();
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
  case FK_Data_8:
    return Value;

    return 0;
  }
}

static unsigned getFixupKindNumBytes(unsigned Kind) {
  switch (Kind) {
  default:
    return 4;
  case FK_Data_1:
    return 1;
  case FK_Data_2:
    return 2;
  case FK_Data_8:
    return 8;
  }
}

namespace {
class SDMAAsmBackend : public MCAsmBackend {
protected:
  const Target &TheTarget;
  bool Is64Bit;

public:
  SDMAAsmBackend(const Target &T)
      : MCAsmBackend(StringRef(T.getName()) == "sparcel" ? support::little
                                                         : support::big),
        TheTarget(T), Is64Bit(StringRef(TheTarget.getName()) == "sparcv9") {}

  unsigned getNumFixupKinds() const override {
    return SDMA::NumTargetFixupKinds;
  }

  std::optional<MCFixupKind> getFixupKind(StringRef Name) const override {
    unsigned Type;
    Type = llvm::StringSwitch<unsigned>(Name)
#define ELF_RELOC(X, Y) .Case(#X, Y)
#include "llvm/BinaryFormat/ELFRelocs/SDMA.def"
#undef ELF_RELOC
               .Default(-1u);
    if (Type == -1u)
      return std::nullopt;
    return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
  }

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override {
    const static MCFixupKindInfo InfosBE[SDMA::NumTargetFixupKinds] = {
        // name                    offset bits  flags
        {"fixup_sparc_call30", 2, 30, MCFixupKindInfo::FKF_IsPCRel},
        {"fixup_sparc_br22", 10, 22, MCFixupKindInfo::FKF_IsPCRel},

    };

    const static MCFixupKindInfo InfosLE[SDMA::NumTargetFixupKinds] = {
        // name                    offset bits  flags
        {"fixup_sdma_abs_14", 0, 30, MCFixupKindInfo::FKF_IsPCRel},
        {"fixup_sdma_rel_8", 0, 22, MCFixupKindInfo::FKF_IsPCRel},
    };

    // Fixup kinds from .reloc directive are like R_SPARC_NONE. They do
    // not require any extra processing.
    if (Kind >= FirstLiteralRelocationKind)
      return MCAsmBackend::getFixupKindInfo(FK_NONE);

    if (Kind < FirstTargetFixupKind)
      return MCAsmBackend::getFixupKindInfo(Kind);

    assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
           "Invalid kind!");
    if (Endian == support::little)
      return InfosLE[Kind - FirstTargetFixupKind];

    return InfosBE[Kind - FirstTargetFixupKind];
  }

  bool shouldForceRelocation(const MCAssembler &Asm, const MCFixup &Fixup,
                             const MCValue &Target) override {
    if (Fixup.getKind() >= FirstLiteralRelocationKind)
      return true;
    not_implemented();
    switch ((SDMA::Fixups)Fixup.getKind()) {
    default:
      return false;
    }
  }

  /// fixupNeedsRelaxation - Target specific predicate for whether a given
  /// fixup requires the associated instruction to be relaxed.
  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override {
    // FIXME.
    llvm_unreachable("fixupNeedsRelaxation() unimplemented");
    return false;
  }
  void relaxInstruction(MCInst &Inst,
                        const MCSubtargetInfo &STI) const override {
    // FIXME.
    llvm_unreachable("relaxInstruction() unimplemented");
  }

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override {
    // Cannot emit NOP with size not multiple of 32 bits.
    if (Count % 4 != 0)
      return false;

    uint64_t NumNops = Count / 4;
    for (uint64_t i = 0; i != NumNops; ++i)
      support::endian::write<uint32_t>(OS, 0x01000000, Endian);

    return true;
  }
};

class ELFSDMAAsmBackend : public SDMAAsmBackend {
  Triple::OSType OSType;

public:
  ELFSDMAAsmBackend(const Target &T, Triple::OSType OSType)
      : SDMAAsmBackend(T), OSType(OSType) {}

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override {

    if (Fixup.getKind() >= FirstLiteralRelocationKind)
      return;
    Value = adjustFixupValue(Fixup.getKind(), Value);
    if (!Value)
      return; // Doesn't change encoding.

    unsigned NumBytes = getFixupKindNumBytes(Fixup.getKind());
    unsigned Offset = Fixup.getOffset();
    // For each byte of the fragment that the fixup touches, mask in the bits
    // from the fixup value. The Value has been "split up" into the
    // appropriate bitfields above.
    for (unsigned i = 0; i != NumBytes; ++i) {
      unsigned Idx = Endian == support::little ? i : (NumBytes - 1) - i;
      Data[Offset + Idx] |= uint8_t((Value >> (i * 8)) & 0xff);
    }
  }

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(OSType);
    return createSDMAELFObjectWriter(Is64Bit, OSABI);
  }
};

} // end anonymous namespace

MCAsmBackend *llvm::createSDMAAsmBackend(const Target &T,
                                         const MCSubtargetInfo &STI,
                                         const MCRegisterInfo &MRI,
                                         const MCTargetOptions &Options) {
  return new ELFSDMAAsmBackend(T, STI.getTargetTriple().getOS());

}
