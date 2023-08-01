#include "SDMARegisterInfo.h"
#include "SDMAFrameLowering.h"
#include "SDMAMachineFunctionInfo.h"
#include "utils.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include <cstdint>

#define GET_REGINFO_TARGET_DESC
#include "SDMAGenRegisterInfo.inc"

namespace llvm {

SDMARegisterInfo::SDMARegisterInfo() : SDMAGenRegisterInfo(
        sdma::RPC,  // Return Address 
        0, // DwarfFlavor
        0, // EHFlavor
        sdma::PC // PC
) {}

const uint16_t *
SDMARegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

const uint32_t *
SDMARegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const {
  return CSR_RegMask;
}

BitVector SDMARegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  markSuperRegs(Reserved, sdma::GP6); // we use gp7 as frame pointer
  markSuperRegs(Reserved, sdma::GP7); // we use gp7 as SP
  return Reserved;
}

bool SDMARegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  not_implemented();
}

Register SDMARegisterInfo::getFrameRegister(const MachineFunction &MF) const {
return getFrameLowering(MF)->hasFP(MF) ? sdma::GP6 : sdma::GP7;
}

} // namespace llvm
