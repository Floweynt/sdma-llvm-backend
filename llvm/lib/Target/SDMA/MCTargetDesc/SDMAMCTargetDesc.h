#pragma once

#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {

class Target;

} // end namespace llvm

namespace llvm {
namespace sdma {
enum OperandType : unsigned {
  OPERAND_FIRST_SDMA_IMM = MCOI::OPERAND_FIRST_TARGET,
  OPERAND_UIMM5,
  OPERAND_UIMM8,
  OPERAND_UIMM14,
  OPERAND_UIMM32,
  OPERAND_SIMM8,
  OPERAND_SIMM32,
};
} // namespace sdma
} // namespace llvm

#define GET_REGINFO_ENUM
#include "SDMAGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "SDMAGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SDMAGenSubtargetInfo.inc"
