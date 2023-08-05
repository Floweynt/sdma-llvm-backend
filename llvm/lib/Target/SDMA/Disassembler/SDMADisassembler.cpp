//===- SDMADisassembler.cpp - Disassembler for SDMA -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is part of the SDMA Disassembler.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SDMAMCTargetDesc.h"
#include "TargetInfo/SDMATargetInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "sparc-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

/// A disassembler class for SDMA.
class SDMADisassembler : public MCDisassembler {
public:
  SDMADisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  virtual ~SDMADisassembler() = default;

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};
} // namespace

static MCDisassembler *createSDMADisassembler(const Target &T,
                                              const MCSubtargetInfo &STI,
                                              MCContext &Ctx) {
  return new SDMADisassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSDMADisassembler() {
  // Register the disassembler.
  TargetRegistry::RegisterMCDisassembler(getTheSDMATarget(),
                                         createSDMADisassembler);
}

static const unsigned GPRegsDecoderTable[] = {
    sdma::GP0, sdma::GP1, sdma::GP2, sdma::GP3,
    sdma::GP4, sdma::GP5, sdma::GP6, sdma::GP7,
};

static DecodeStatus DecodeGPRegsRegisterClass(MCInst &Inst, unsigned RegNo,
                                              uint64_t Address,
                                              const MCDisassembler *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;
  unsigned Reg = GPRegsDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeSImmOperand(MCInst &Inst, uint32_t Imm,
                                      int64_t Address,
                                      const MCDisassembler *Decoder) {
  assert(isInt<N>(Imm) && "Invalid immediate");
  // Sign-extend the number in the bottom N bits of Imm
  Inst.addOperand(MCOperand::createImm(SignExtend64<N>(Imm)));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeUImmOperand(MCInst &Inst, uint32_t Imm,
                                      int64_t Address,
                                      const MCDisassembler *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  Inst.addOperand(MCOperand::createImm(Imm));
  return MCDisassembler::Success;
}

#include "SDMAGenDisassemblerTables.inc"

/// Read four bytes from the ArrayRef and return 32 bit word.
static DecodeStatus readInstruction16(ArrayRef<uint8_t> Bytes, uint64_t Address,
                                      uint64_t &Size, uint32_t &Insn,
                                      bool IsLittleEndian) {
  // We want to read exactly 2 Bytes of data.
  if (Bytes.size() < 2) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Insn = IsLittleEndian ? (Bytes[0] << 0) | (Bytes[1] << 8)
                        : (Bytes[1] << 0) | (Bytes[0] << 8);

  return MCDisassembler::Success;
}

DecodeStatus SDMADisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                              ArrayRef<uint8_t> Bytes,
                                              uint64_t Address,
                                              raw_ostream &CStream) const {
  uint32_t Insn;
  DecodeStatus Result = readInstruction16(Bytes, Address, Size, Insn, true);
  if (Result == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Calling the auto-generated decoder function.

  Result = decodeInstruction(DecoderTable16, Instr, Insn, Address, this, STI);

  if (Result != MCDisassembler::Fail) {
    Size = 2;
    return Result;
  }

  return MCDisassembler::Fail;
}

