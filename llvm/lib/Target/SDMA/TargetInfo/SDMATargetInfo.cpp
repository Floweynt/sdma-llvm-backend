
#include "TargetInfo/SDMATargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheSDMATarget() {
  static Target TheTarget;
  return TheTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSDMATargetInfo() {
  RegisterTarget<Triple::sdma, /*HasJIT=*/false> X(getTheSDMATarget(),
                                                    "sdma", "SDMA", "SDMA");
}
