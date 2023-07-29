#include "SDMAISelLowering.h"
#include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "SDMARegisterInfo.h"
#include "SDMASubtarget.h"
#include "utils.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/MachineValueType.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"
#include <csignal>
#include <iostream>

using namespace llvm;

#include "SDMAGenCallingConv.inc"

SDMATargetLowering::SDMATargetLowering(const TargetMachine &TM,
                                       const SDMASubtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {
  addRegisterClass(MVT::i32, &sdma::GPRegsRegClass);
  setMinFunctionAlignment(Align(4));
  computeRegisterProperties(Subtarget->getRegisterInfo());

}

SDValue SDMATargetLowering::LowerOperation(SDValue Op,
                                           SelectionDAG &DAG) const {
  const SDNode *N = Op.getNode();
  EVT VT = Op.getValueType();
  SDLoc DL(N);


  return Op;
  /*
  if (Op.getOpcode() == ISD::ADD) {
    assert(N->getNumOperands() == 2 && "my assumption was false... :(");
    std::cout << "lowering add\n";
    auto *Lhs = N->getOperand(0).getNode();
    auto *Rhs = N->getOperand(1).getNode();
    if ((Lhs->getOpcode() == ISD::Constant) !=
        (Rhs->getOpcode() == ISD::Constant)) {
      auto *ConstNode =
          (ConstantSDNode *)(Lhs->getOpcode() == ISD::Constant ? Lhs : Rhs);
      if (ConstNode->getSExtValue() < 0) {
        return DAG.getNode(
            ISD::SUB, DL, MVT::i32,
            N->getOperand(Lhs->getOpcode() == ISD::Constant ? 1 : 0),
            DAG.getConstant(-ConstNode->getSExtValue(), DL, MVT::i32));
      }
    }
  }
  if(Op.getOpcode() == ISD::SHL) {
    assert(N->getNumOperands() == 2 && "my assumption was false... :(");
    std::cout << "lowering shl\n";
    auto *Lhs = N->getOperand(0).getNode();
    auto *Rhs = N->getOperand(1).getNode();

    if ((Lhs->getOpcode() == ISD::Constant) !=
        (Rhs->getOpcode() == ISD::Constant)) {
      auto *ConstNode =
          (ConstantSDNode *)(Lhs->getOpcode() == ISD::Constant ? Lhs : Rhs);

      auto Amount = ConstNode->getZExtValue();
      if(Amount == 1)
          return Op;

      SDValue Shiftee = N->getOperand(Lhs->getOpcode() == ISD::Constant ? 1 : 0);
        
      for(size_t I = 0; I < Amount; I++) {
        Shiftee = DAG.getNode(
            ISD::SHL, DL, MVT::i32,
            Shiftee,
            DAG.getConstant(1, DL, MVT::i32));
      }

      return Shiftee;
    }

  }


  return Op;*/
}

bool SDMATargetLowering::useSoftFloat() const { return true; }

/// computeKnownBitsForTargetNode - Determine which of the bits specified
/// in Mask are known to be either zero or one and return them in the
/// KnownZero/KnownOne bitsets.
void SDMATargetLowering::computeKnownBitsForTargetNode(
    const SDValue Op, KnownBits &Known, const APInt &DemandedElts,
    const SelectionDAG &DAG, unsigned Depth) const {
  not_implemented();
}

MachineBasicBlock *
SDMATargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                                MachineBasicBlock *MBB) const {
  not_implemented();
}

const char *SDMATargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((SDMAISD::NodeType)Opcode) {
  case SDMAISD::RET:
    return " SDMAISD::RET";
  case SDMAISD::CALL:
    return " SDMAISD::CALL";
  case SDMAISD::FIRST_NUMBER:
    break;
  }
  return nullptr;
}

SDMATargetLowering::ConstraintType
SDMATargetLowering::getConstraintType(StringRef Constraint) const {
  not_implemented();
}

llvm::SDMATargetLowering::ConstraintWeight
SDMATargetLowering::getSingleConstraintMatchWeight(
    AsmOperandInfo &info, const char *constraint) const {
  not_implemented();
}

void SDMATargetLowering::LowerAsmOperandForConstraint(SDValue Op,
                                                      std::string &Constraint,
                                                      std::vector<SDValue> &Ops,
                                                      SelectionDAG &DAG) const {
  not_implemented();
}

std::pair<unsigned, const TargetRegisterClass *>
SDMATargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                 StringRef Constraint,
                                                 MVT VT) const {

  not_implemented();
}

bool SDMATargetLowering::isOffsetFoldingLegal(
    const GlobalAddressSDNode *GA) const {
  not_implemented();
}

Register
SDMATargetLowering::getRegisterByName(const char *RegName, LLT VT,
                                      const MachineFunction &MF) const {
  not_implemented();
}

EVT SDMATargetLowering::getSetCCResultType(const DataLayout &DL,
                                           LLVMContext &Context, EVT VT) const {
  not_implemented();
}

SDValue SDMATargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_SDMA);

  const unsigned StackOffset = 92;

  unsigned InIdx = 0;
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i, ++InIdx) {
    CCValAssign &VA = ArgLocs[i];

    if (Ins[InIdx].Flags.isSRet()) {
      assert(false && "what the fuck is a sret");
      continue;
    }

    if (VA.isRegLoc()) {
      /*
    if (VA.needsCustom()) {
      assert(VA.getLocVT() == MVT::f64 || VA.getLocVT() == MVT::v2i32);

      Register VRegHi = RegInfo.createVirtualRegister(&SP::IntRegsRegClass);
      MF.getRegInfo().addLiveIn(VA.getLocReg(), VRegHi);
      SDValue HiVal = DAG.getCopyFromReg(Chain, dl, VRegHi, MVT::i32);

      assert(i+1 < e);
      CCValAssign &NextVA = ArgLocs[++i];

      SDValue LoVal;
      if (NextVA.isMemLoc()) {
        int FrameIdx = MF.getFrameInfo().
          CreateFixedObject(4, StackOffset+NextVA.getLocMemOffset(),true);
        SDValue FIPtr = DAG.getFrameIndex(FrameIdx, MVT::i32);
        LoVal = DAG.getLoad(MVT::i32, dl, Chain, FIPtr, MachinePointerInfo());
      } else {
        Register loReg = MF.addLiveIn(NextVA.getLocReg(),
                                      &sdma::GPRegsRegClass);
        LoVal = DAG.getCopyFromReg(Chain, dl, loReg, MVT::i32);
      }

      if (IsLittleEndian)
        std::swap(LoVal, HiVal);

      SDValue WholeValue =
        DAG.getNode(ISD::BUILD_PAIR, dl, MVT::i64, LoVal, HiVal);
      WholeValue = DAG.getNode(ISD::BITCAST, dl, VA.getLocVT(), WholeValue);
      InVals.push_back(WholeValue);
      continue;
    }*/
      Register VReg = RegInfo.createVirtualRegister(&sdma::GPRegsRegClass);
      MF.getRegInfo().addLiveIn(VA.getLocReg(), VReg);
      SDValue Arg = DAG.getCopyFromReg(Chain, dl, VReg, MVT::i32);

      if (VA.getLocVT() != MVT::i32) {
        assert(false && "please have i32 arguments, i dont want to implement "
                        "anything else...");
      }

      InVals.push_back(Arg);
      continue;
    }

    assert(false && "failed");
    assert(VA.isMemLoc());

    unsigned Offset = VA.getLocMemOffset() + StackOffset;
    auto PtrVT = getPointerTy(DAG.getDataLayout());

    /*
    if (VA.needsCustom()) {
      assert(VA.getValVT() == MVT::f64 || VA.getValVT() == MVT::v2i32);
      // If it is double-word aligned, just load.
      if (Offset % 8 == 0) {
        int FI = MF.getFrameInfo().CreateFixedObject(8,
                                                     Offset,
                                                     true);
        SDValue FIPtr = DAG.getFrameIndex(FI, PtrVT);
        SDValue Load =
            DAG.getLoad(VA.getValVT(), dl, Chain, FIPtr, MachinePointerInfo());
        InVals.push_back(Load);
        continue;
      }

      int FI = MF.getFrameInfo().CreateFixedObject(4,
                                                   Offset,
                                                   true);
      SDValue FIPtr = DAG.getFrameIndex(FI, PtrVT);
      SDValue HiVal =
          DAG.getLoad(MVT::i32, dl, Chain, FIPtr, MachinePointerInfo());
      int FI2 = MF.getFrameInfo().CreateFixedObject(4,
                                                    Offset+4,
                                                    true);
      SDValue FIPtr2 = DAG.getFrameIndex(FI2, PtrVT);

      SDValue LoVal =
          DAG.getLoad(MVT::i32, dl, Chain, FIPtr2, MachinePointerInfo());

      if (IsLittleEndian)
        std::swap(LoVal, HiVal);

      SDValue WholeValue =
        DAG.getNode(ISD::BUILD_PAIR, dl, MVT::i64, LoVal, HiVal);
      WholeValue = DAG.getNode(ISD::BITCAST, dl, VA.getValVT(), WholeValue);
      InVals.push_back(WholeValue);
      continue;
    }

    int FI = MF.getFrameInfo().CreateFixedObject(4,
                                                 Offset,
                                                 true);
    SDValue FIPtr = DAG.getFrameIndex(FI, PtrVT);
    SDValue Load ;
    if (VA.getValVT() == MVT::i32 || VA.getValVT() == MVT::f32) {
      Load = DAG.getLoad(VA.getValVT(), dl, Chain, FIPtr, MachinePointerInfo());
    } else if (VA.getValVT() == MVT::f128) {
      report_fatal_error("SPARCv8 does not handle f128 in calls; "
                         "pass indirectly");
    } else {
      // We shouldn't see any other value types here.
      llvm_unreachable("Unexpected ValVT encountered in frame lowering.");
    }
    InVals.push_back(Load);*/
  }

  if (MF.getFunction().hasStructRetAttr()) {
    not_implemented();
    /*
  // Copy the SRet Argument to SRetReturnReg.
  SDMAMachineFunctionInfo *SFI = MF.getInfo<SparcMachineFunctionInfo>();
  Register Reg = SFI->getSRetReturnReg();
  if (!Reg) {
    Reg = MF.getRegInfo().createVirtualRegister(&SP::IntRegsRegClass);
    SFI->setSRetReturnReg(Reg);
  }
  SDValue Copy = DAG.getCopyToReg(DAG.getEntryNode(), dl, Reg, InVals[0]);
  Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, Copy, Chain);*/
  }

  // Store remaining ArgRegs to the stack if this is a varargs function.
  if (isVarArg) {
    not_implemented();
    /*
  static const MCPhysReg ArgRegs[] = {
    SP::I0, SP::I1, SP::I2, SP::I3, SP::I4, SP::I5
  };
  unsigned NumAllocated = CCInfo.getFirstUnallocated(ArgRegs);
  const MCPhysReg *CurArgReg = ArgRegs+NumAllocated, *ArgRegEnd = ArgRegs+6;
  unsigned ArgOffset = CCInfo.getStackSize();
  if (NumAllocated == 6)
    ArgOffset += StackOffset;
  else {
    assert(!ArgOffset);
    ArgOffset = 68+4*NumAllocated;
  }

  // Remember the vararg offset for the va_start implementation.
  FuncInfo->setVarArgsFrameOffset(ArgOffset);

  std::vector<SDValue> OutChains;

  for (; CurArgReg != ArgRegEnd; ++CurArgReg) {
    Register VReg = RegInfo.createVirtualRegister(&SP::IntRegsRegClass);
    MF.getRegInfo().addLiveIn(*CurArgReg, VReg);
    SDValue Arg = DAG.getCopyFromReg(DAG.getRoot(), dl, VReg, MVT::i32);

    int FrameIdx = MF.getFrameInfo().CreateFixedObject(4, ArgOffset,
                                                       true);
    SDValue FIPtr = DAG.getFrameIndex(FrameIdx, MVT::i32);

    OutChains.push_back(
        DAG.getStore(DAG.getRoot(), dl, Arg, FIPtr, MachinePointerInfo()));
    ArgOffset += 4;
  }

  if (!OutChains.empty()) {
    OutChains.push_back(Chain);
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, OutChains);
  }*/
  }

  return Chain;
}

SDValue SDMATargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                      SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &IsTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool IsVarArg = CLI.IsVarArg;
  MachineFunction &MF = DAG.getMachineFunction();

  IsTailCall = false;

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  const Function *F = nullptr;
  if (const GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    const GlobalValue *GV = G->getGlobal();
    if (isa<Function>(GV))
      F = cast<Function>(GV);
    Callee =
        DAG.getTargetGlobalAddress(GV, DL, getPointerTy(DAG.getDataLayout()));
  } else if (const ExternalSymbolSDNode *ES =
                 dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(ES->getSymbol(),
                                         getPointerTy(DAG.getDataLayout()));
  }

  CCInfo.AnalyzeCallOperands(Outs, CC_SDMA);
  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getStackSize();

  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;

  // First, walk the register assignments, inserting copies.
  unsigned AI, AE;
  bool HasStackArgs = false;
  for (AI = 0, AE = ArgLocs.size(); AI != AE; ++AI) {
    CCValAssign &VA = ArgLocs[AI];
    EVT RegVT = VA.getLocVT();
    SDValue Arg = OutVals[AI];

    // Promote the value if needed. With Clang this should not happen.
    switch (VA.getLocInfo()) {
    default:
      llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full:
      break;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, DL, RegVT, Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, DL, RegVT, Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, DL, RegVT, Arg);
      break;
    case CCValAssign::BCvt:
      Arg = DAG.getNode(ISD::BITCAST, DL, RegVT, Arg);
      break;
    }

    // Stop when we encounter a stack argument, we need to process them
    // in reverse order in the loop below.
    if (VA.isMemLoc()) {
      HasStackArgs = true;
      break;
    }

    // Arguments that can be passed on registers must be kept in the RegsToPass
    // vector.
    RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
  }

  // Second, stack arguments have to walked.
  // Previously this code created chained stores but those chained stores appear
  // to be unchained in the legalization phase. Therefore, do not attempt to
  // chain them here. In fact, chaining them here somehow causes the first and
  // second store to be reversed which is the exact opposite of the intended
  // effect.
  if (HasStackArgs) {
    SmallVector<SDValue, 8> MemOpChains;
    for (; AI != AE; AI++) {
      CCValAssign &VA = ArgLocs[AI];
      SDValue Arg = OutVals[AI];

      assert(VA.isMemLoc());

      // SP points to one stack slot further so add one to adjust it.
      SDValue PtrOff = DAG.getNode(
          ISD::ADD, DL, getPointerTy(DAG.getDataLayout()),
          DAG.getRegister(sdma::GP7, getPointerTy(DAG.getDataLayout())),
          DAG.getIntPtrConstant(VA.getLocMemOffset() + 1, DL));

      MemOpChains.push_back(
          DAG.getStore(Chain, DL, Arg, PtrOff,
                       MachinePointerInfo::getStack(MF, VA.getLocMemOffset())));
    }

    if (!MemOpChains.empty())
      Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);
  }

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InGlue in
  // necessary since all emited instructions must be stuck together.
  SDValue InGlue;
  for (auto Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, Reg.first, Reg.second, InGlue);
    InGlue = Chain.getValue(1);
  }

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are known live
  // into the call.
  for (auto Reg : RegsToPass) {
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));
  }

  // The zero register (usually R1) must be passed as an implicit register so
  // that this register is correctly zeroed in interrupts.

  // Add a register mask operand representing the call-preserved registers.
  const TargetRegisterInfo *TRI = Subtarget->getRegisterInfo();
  const uint32_t *Mask =
      TRI->getCallPreservedMask(DAG.getMachineFunction(), CallConv);
  assert(Mask && "Missing call preserved mask for calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (InGlue.getNode()) {
    Ops.push_back(InGlue);
  }

  Chain = DAG.getNode(SDMAISD::CALL, DL, NodeTys, Ops);
  InGlue = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain, NumBytes, 0, InGlue, DL);

  if (!Ins.empty()) {
    InGlue = Chain.getValue(1);
  }

  {
    SmallVector<CCValAssign, 16> RVLocs;
    CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                   *DAG.getContext());
    CCInfo.AnalyzeCallResult(Ins, RetCC_SDMA);
    // Copy all of the result registers out of their specified physreg.
    for (CCValAssign const &RVLoc : RVLocs) {
      Chain = DAG.getCopyFromReg(Chain, DL, RVLoc.getLocReg(), RVLoc.getValVT(),
                                 InGlue)
                  .getValue(1);
      InGlue = Chain.getValue(2);
      InVals.push_back(Chain.getValue(0));
    }

    return Chain;
  }
}

bool SDMATargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool isVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context) const {
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);
  return CCInfo.CheckReturn(Outs, RetCC_SDMA);
}

SDValue
SDMATargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                bool IsVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                const SDLoc &DL, SelectionDAG &DAG) const {
  // TODO: implement this correctly
  MachineFunction &MF = DAG.getMachineFunction();
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_SDMA);
  SDValue Glue;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  for (unsigned i = 0, realRVLocIdx = 0; i != RVLocs.size();
       ++i, ++realRVLocIdx) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    SDValue Arg = OutVals[realRVLocIdx];

    if (VA.needsCustom()) {
      not_implemented();
    } else {
      Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Arg, Glue);
    }

    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  unsigned RetAddrOffset = 8;
  if (MF.getFunction().hasStructRetAttr()) {
    not_implemented();
  }

  RetOps[0] = Chain;

  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(SDMAISD::RET, DL, MVT::Other, RetOps);
}

SDValue SDMATargetLowering::PerformDAGCombine(SDNode *N,
                                              DAGCombinerInfo &DCI) const {
  return SDValue();
}

llvm::SDMATargetLowering::AtomicExpansionKind
SDMATargetLowering::shouldExpandAtomicRMWInIR(AtomicRMWInst *AI) const {
  not_implemented();
}

void SDMATargetLowering::ReplaceNodeResults(SDNode *N,
                                            SmallVectorImpl<SDValue> &Results,
                                            SelectionDAG &DAG) const {
  not_implemented();
}
