#pragma once

#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {

class formatted_raw_ostream;

class SDMATargetStreamer : public MCTargetStreamer {
  virtual void anchor();

public:
  SDMATargetStreamer(MCStreamer &S);
  virtual void emitSDMARegisterIgnore(unsigned Reg){};
  virtual void emitSDMARegisterScratch(unsigned Reg){};
};

class SDMATargetAsmStreamer : public SDMATargetStreamer {
  formatted_raw_ostream &OS;

public:
  SDMATargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);
  void emitSDMARegisterIgnore(unsigned Reg) override;
  void emitSDMARegisterScratch(unsigned Reg) override;
};

// This part is for ELF object output
class SDMATargetELFStreamer : public SDMATargetStreamer {
public:
  SDMATargetELFStreamer(MCStreamer &S);
  MCELFStreamer &getStreamer();
  void emitSDMARegisterIgnore(unsigned Reg) override {}
  void emitSDMARegisterScratch(unsigned Reg) override {}
};
} // end namespace llvm
