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
  OPERAND_UIMM2,
  OPERAND_UIMM3,
  OPERAND_UIMM5,
  OPERAND_UIMM8,
  OPERAND_UIMM14,
  OPERAND_UIMM32,
  OPERAND_SIMM8,
  OPERAND_SIMM32,
  OPERAND_CONDCODE,
  OPERAND_BRTARGET
};
} // namespace sdma
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;

MCCodeEmitter *createSDMAMCCodeEmitter(const MCInstrInfo &MCII,
                                        MCContext &Ctx);
MCAsmBackend *createSDMAAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const MCTargetOptions &Options);
std::unique_ptr<MCObjectTargetWriter> createSDMAELFObjectWriter(bool Is64Bit,
                                                                 uint8_t OSABI);
} // namespace llvm

#define GET_REGINFO_ENUM
#include "SDMAGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "SDMAGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SDMAGenSubtargetInfo.inc"
