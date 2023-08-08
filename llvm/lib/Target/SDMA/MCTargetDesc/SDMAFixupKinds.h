#pragma once

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace SDMA {
enum Fixups {
  fixup_sdma_abs_14 = FirstTargetFixupKind,
  fixup_sdma_rel_8,
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
} // namespace SDMA
} // namespace llvm
