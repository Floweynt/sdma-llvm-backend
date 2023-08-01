#include "SDMATargetMachine.h"
#include "utils.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "sdma-isel"
#define PASS_NAME "SDMA DAG->DAG Pattern Instruction Selection"

namespace {
class SDMADAGToDAGISel : public SelectionDAGISel {
  const SDMASubtarget *Subtarget = nullptr;

public:
  static char ID;

  SDMADAGToDAGISel() = delete;

  explicit SDMADAGToDAGISel(SDMATargetMachine &tm) : SelectionDAGISel(ID, tm) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    Subtarget = &MF.getSubtarget<SDMASubtarget>();
    return SelectionDAGISel::runOnMachineFunction(MF);
  }

  void Select(SDNode *N) override;

  bool SelectADDRrr(SDValue N, SDValue &R1, SDValue &R2);
  bool SelectADDRri(SDValue N, SDValue &Base, SDValue &Offset);

  bool SelectInlineAsmMemoryOperand(const SDValue &Op, unsigned ConstraintID,
                                    std::vector<SDValue> &OutOps) override;

#include "SDMAGenDAGISel.inc"

private:
  SDNode *getGlobalBaseReg();
  bool tryInlineAsm(SDNode *N);
};
} // end anonymous namespace

char SDMADAGToDAGISel::ID = 0;

INITIALIZE_PASS(SDMADAGToDAGISel, DEBUG_TYPE, PASS_NAME, false, false)

SDNode *SDMADAGToDAGISel::getGlobalBaseReg() {
  Register GlobalBaseReg = Subtarget->getInstrInfo()->getGlobalBaseReg(MF);
  return CurDAG
      ->getRegister(GlobalBaseReg, TLI->getPointerTy(CurDAG->getDataLayout()))
      .getNode();
}

bool SDMADAGToDAGISel::SelectADDRri(SDValue Addr, SDValue &Base,
                                    SDValue &Offset) {
  not_implemented();
}

bool SDMADAGToDAGISel::SelectADDRrr(SDValue Addr, SDValue &R1, SDValue &R2) {
  not_implemented();
}

bool SDMADAGToDAGISel::tryInlineAsm(SDNode *N) { not_implemented(); }

void SDMADAGToDAGISel::Select(SDNode *N) {
  SDLoc Dl(N);
  if (N->isMachineOpcode()) {
    N->setNodeId(-1);
    return;
  }

  switch (N->getOpcode()) {
  default:
    break;
  case ISD::INLINEASM:
  case ISD::INLINEASM_BR: {
    not_implemented();
    if (tryInlineAsm(N))
      return;
    break;
  }
  }

  SelectCode(N);
}

void validateSelectionDag(SelectionDAG* DAG)
{
  SmallVector<SDNode*, 64> Worklist;
  SmallPtrSet<SDNode*, 32> Visited;
  Worklist.push_back(DAG->getRoot().getNode());
  Visited.insert(DAG->getRoot().getNode());

  SmallVector<SUnit*, 8> CallSUnits;
  while (!Worklist.empty()) {
    SDNode *NI = Worklist.pop_back_val();
    for (const SDValue &Op : NI->op_values())
      if (Visited.insert(Op.getNode()).second)
        Worklist.push_back(Op.getNode());

    SDNode *N = NI;
    while (N->getNumOperands() &&
           N->getOperand(N->getNumOperands()-1).getValueType() == MVT::Glue) {
      N = N->getOperand(N->getNumOperands()-1).getNode();
    }

  }
}

bool SDMADAGToDAGISel::SelectInlineAsmMemoryOperand(
    const SDValue &Op, unsigned ConstraintID, std::vector<SDValue> &OutOps) {
  not_implemented();
  SDValue Op0, Op1;
  switch (ConstraintID) {
  default:
    return true;
  case InlineAsm::Constraint_o:
  case InlineAsm::Constraint_m: // memory
    if (!SelectADDRrr(Op, Op0, Op1))
      SelectADDRri(Op, Op0, Op1);
    break;
  }

  OutOps.push_back(Op0);
  OutOps.push_back(Op1);
  return false;
}

FunctionPass *llvm::createSDMAISelDag(SDMATargetMachine &TM) {
  return new SDMADAGToDAGISel(TM);
}
