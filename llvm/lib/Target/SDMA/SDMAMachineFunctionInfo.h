#pragma once

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

  class SDMAMachineFunctionInfo : public MachineFunctionInfo {
    Register GlobalBaseReg;
    int VarArgsFrameOffset;
    Register SRetReturnReg;
    bool IsLeafProc;
  public:
    SDMAMachineFunctionInfo()
      : GlobalBaseReg(0), VarArgsFrameOffset(0), SRetReturnReg(0),
        IsLeafProc(false) {}
    SDMAMachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI)
        : GlobalBaseReg(0), VarArgsFrameOffset(0), SRetReturnReg(0),
          IsLeafProc(false) {}

    MachineFunctionInfo *
    clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
          const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
        const override;

    Register getGlobalBaseReg() const { return GlobalBaseReg; }
    void setGlobalBaseReg(Register Reg) { GlobalBaseReg = Reg; }

    int getVarArgsFrameOffset() const { return VarArgsFrameOffset; }
    void setVarArgsFrameOffset(int Offset) { VarArgsFrameOffset = Offset; }

    Register getSRetReturnReg() const { return SRetReturnReg; }
    void setSRetReturnReg(Register Reg) { SRetReturnReg = Reg; }

    void setLeafProc(bool rhs) { IsLeafProc = rhs; }
    bool isLeafProc() const { return IsLeafProc; }
  };
}
