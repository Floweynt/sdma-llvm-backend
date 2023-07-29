#include "SDMATargetMachine.h"
#include "SDMA.h"
#include "SDMAMachineFunctionInfo.h"
#include "SDMATargetObjectFile.h"
#include "TargetInfo/SDMATargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"
#include <optional>

using namespace llvm;

static CodeModel::Model

getEffectiveSDMACodeModel(std::optional<CodeModel::Model> CM) {
  if (CM) {
    if (*CM == CodeModel::Tiny)
      report_fatal_error("Target does not support the tiny CodeModel", false);
    if (*CM == CodeModel::Kernel)
      report_fatal_error("Target does not support the kernel CodeModel", false);
    return *CM;
  }
  return CodeModel::Small;
}

namespace {
/// SDMA Code Generator Pass Configuration Options.
class SDMAPassConfig : public TargetPassConfig {
public:
  SDMAPassConfig(SDMATargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  SDMATargetMachine &getSDMATargetMachine() const {
    return getTM<SDMATargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
  void addPreSched2() override;
};
} // namespace

TargetPassConfig *SDMATargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SDMAPassConfig(*this, PM);
}

void SDMAPassConfig::addIRPasses() {
  addPass(createAtomicExpandPass());
  TargetPassConfig::addIRPasses();
}

bool SDMAPassConfig::addInstSelector() {
  addPass(createSDMAISelDag(getSDMATargetMachine()));
  return false;
}

void SDMAPassConfig::addPreEmitPass() {}

void SDMAPassConfig::addPreSched2() {
  addPass(createSDMAExpandPseudoPass());
}

SDMATargetMachine::SDMATargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     std::optional<Reloc::Model> RM,
                                     std::optional<CodeModel::Model> CM,
                                     CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, "e-p:32:32-i32:32", TT, CPU, FS, Options,
                        RM.value_or(Reloc::Static),
                        getEffectiveSDMACodeModel(CM), OL),
      Subtarget(TT, CPU, FS, *this),
      TLOF(std::make_unique<SDMAELFTargetObjectFile>()) {
  // I dont know
  AsmInfo.reset(new MCAsmInfo());
}

const SDMASubtarget *
SDMATargetMachine::getSubtargetImpl(const Function &F) const {
  return &Subtarget;
}

MachineFunctionInfo *SDMATargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return SDMAMachineFunctionInfo::create<SDMAMachineFunctionInfo>(Allocator, F,
                                                                  STI);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSDMATarget() {
  // Register the target.
  RegisterTargetMachine<llvm::SDMATargetMachine> X(getTheSDMATarget());

  // PassRegistry &PR = *PassRegistry::getPassRegistry();
  // initializeSDMADAGToDAGISelPass(PR);
}

