//===-- SDMATargetStreamer.cpp - SDMA Target Streamer Methods -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides SDMA specific target streamer methods.
//
//===----------------------------------------------------------------------===//

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

void SDMATargetAsmStreamer::emitSDMARegisterIgnore(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(SDMAInstPrinter::getRegisterName(reg)).lower()
     << ", #ignore\n";
}

void SDMATargetAsmStreamer::emitSDMARegisterScratch(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(SDMAInstPrinter::getRegisterName(reg)).lower()
     << ", #scratch\n";
}

SDMATargetELFStreamer::SDMATargetELFStreamer(MCStreamer &S)
    : SDMATargetStreamer(S) {}

MCELFStreamer &SDMATargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
