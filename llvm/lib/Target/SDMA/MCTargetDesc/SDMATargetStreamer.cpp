#include "SDMATargetStreamer.h"
#include "SDMAInstPrinter.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

// pin vtable to this file
SDMATargetStreamer::SDMATargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

void SDMATargetStreamer::anchor() {}

SDMATargetAsmStreamer::SDMATargetAsmStreamer(MCStreamer &S,
                                             formatted_raw_ostream &OS)
    : SDMATargetStreamer(S), OS(OS) {}

void SDMATargetAsmStreamer::emitSDMARegisterIgnore(unsigned Reg) {
  OS << "\t.register "
     << "%" << StringRef(SDMAInstPrinter::getRegisterName(Reg)).lower()
     << ", #ignore\n";
}

void SDMATargetAsmStreamer::emitSDMARegisterScratch(unsigned Reg) {
  OS << "\t.register "
     << "%" << StringRef(SDMAInstPrinter::getRegisterName(Reg)).lower()
     << ", #scratch\n";
}

SDMATargetELFStreamer::SDMATargetELFStreamer(MCStreamer &S)
    : SDMATargetStreamer(S) {}

MCELFStreamer &SDMATargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
