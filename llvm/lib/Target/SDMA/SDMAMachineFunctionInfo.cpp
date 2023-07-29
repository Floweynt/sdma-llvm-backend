#include "SDMAMachineFunctionInfo.h"

using namespace llvm;

MachineFunctionInfo *SDMAMachineFunctionInfo::clone(
    BumpPtrAllocator &Allocator, MachineFunction &DestMF,
    const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
    const {
  return DestMF.cloneInfo<SDMAMachineFunctionInfo>(*this);
}
