// SelectionDAG stuff

include "SDMAInstrFormats.td"

def SDT_call : SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>;
def SDT_callseq_start : SDCallSeqStart<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_callseq_end : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_cmp : SDTypeProfile<0, 3, [SDTCisSameAs<0, 1>, SDTCisVT<2, i32>]>;
def SDT_br : SDTypeProfile<0, 1, [SDTCisVT<0, OtherVT>]>;

def ret : SDNode<"SDMAISD::RET", SDTNone,
                 [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def call : SDNode<"SDMAISD::CALL", SDT_call,
                  [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_callseq_start,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def callseq_end : SDNode<"ISD::CALLSEQ_END", SDT_callseq_end,
                         [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def sdma_cmp : SDNode<"SDMAISD::CMP", SDT_cmp, [SDNPOutGlue]>;
def sdma_brt : SDNode<"SDMAISD::BR_T", SDT_br, [SDNPHasChain, SDNPInGlue]>;
def sdma_brf : SDNode<"SDMAISD::BR_F", SDT_br, [SDNPHasChain, SDNPInGlue]>;

// Target generic defs

class SDMAOperand<string name, ValueType vt> : Operand<vt> {
  let OperandType = name;
  let OperandNamespace = "sdma";
}

// Operands
class ImmAsmOperand<string prefix, int width, string suffix = "">
    : AsmOperandClass {
  let Name = prefix #"Imm" #width #suffix;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid ", Name);
}

def SDMAMEMrAsmOperand : AsmOperandClass {
  let Name = "MEMr";
  let ParserMethod = "parseMEMOperand";
}

def SDMAMEMriAsmOperand : AsmOperandClass {
  let Name = "MEMri";
  let ParserMethod = "parseMEMOperand";
}

//...

class uimm<int width>
    : SDMAOperand<!strconcat("OPERAND_UIMM", !cast<string>(width)), i32>,
      ImmLeaf<i32,
              !strconcat("return isUInt<", !cast<string>(width), ">(Imm);")> {
  let ParserMatchClass = ImmAsmOperand<"U", width>;
  let EncoderMethod = "encodeImm";
  let DecoderMethod =
      !strconcat("decodeUImmOperand<", !cast<string>(width), ">");
  let MCOperandPredicate = !strconcat([{
                                        int64_t Imm;
                                        if (MCOp.evaluateAsConstantImm(Imm))
                                          return isUInt <
                                      }],
                                      !cast<string>(width), [{
                                        > (Imm);
                                        return MCOp.isBareSymbolRef();
                                      }]);
}

class simm<int width>
    : SDMAOperand<!strconcat("OPERAND_SIMM", !cast<string>(width)), i32>,
      ImmLeaf<i32,
              !strconcat("return isInt<", !cast<string>(width), ">(Imm);")> {
  let ParserMatchClass = ImmAsmOperand<"S", width>;
  let EncoderMethod = "encodeImm";
  let DecoderMethod =
      !strconcat("decodeSImmOperand<", !cast<string>(width), ">");
  let MCOperandPredicate = !strconcat([{
                                        int64_t Imm;
                                        if (MCOp.evaluateAsConstantImm(Imm))
                                          return isSInt <
                                      }],
                                      !cast<string>(width), [{
                                        > (Imm);
                                        return MCOp.isBareSymbolRef();
                                      }]);
}

// concrete operand defs
def uimm2 : uimm<2>;
def uimm3 : uimm<3>;
def uimm5 : uimm<5>;
def uimm8 : uimm<8>;
def uimm14 : uimm<14>;
def uimm32 : uimm<32>;
def simm8 : simm<8>;
def simm32 : simm<32>;

def MEMri : Operand<iPTR> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops GPRegs, uimm8);
  let ParserMatchClass = SDMAMEMriAsmOperand;
}

def rel_brtarget : SDMAOperand<"OPERAND_REL_BRTARGET", OtherVT> {
  let PrintMethod = "printRelBranchTarget";
  let EncoderMethod = "encodeRelBranchTarget";
  let DecoderMethod = "decodeRelBranchTarget";
}

def abs_brtarget : SDMAOperand<"OPERAND_ABS_BRTARGET", OtherVT> {
  let PrintMethod = "printAbsBranchTarget";
  let EncoderMethod = "encodeAbsBranchTarget";
  let DecoderMethod = "decodeAbsBranchTarget";
}

def condcode : SDMAOperand<"OPERAND_CONDCODE", i32> {
  let PrintMethod = "printCondType";
  let EncoderMethod = "encodeCondType";
}

// Patterns
def ADDRri : ComplexPattern<iPTR, 2, "SelectADDRri", [frameindex], []>;

// Instruction types
multiclass ArithInstr<bits<4> opc, string asm, SDNode opNode> {
  def rr : RRI<opc,
               (outs GPRegs
                : $dest),
               (ins GPRegs
                : $sdest, GPRegs
                : $src),
               !strconcat(asm, " $dest, $src"), [(set GPRegs
                                                  : $dest, (opNode GPRegs
                                                            : $sdest, GPRegs
                                                            : $src))]> {
    let Constraints = "$sdest = $dest";
  }
  def ri : RII<opc,
               (outs GPRegs
                : $dest),
               (ins GPRegs
                : $sdest, uimm8
                : $imm),
               !strconcat(asm, " $dest, $imm"), [(set GPRegs
                                                  : $dest, (opNode GPRegs
                                                            : $sdest, uimm8
                                                            : $imm))]> {
    let Constraints = "$sdest = $dest";
  }
}

class ArithUnaryInstr<bits<5> opc, string asm, list<dag> pattern>
    : UnaryI<opc,
             (outs GPRegs
              : $dest),
             (ins GPRegs
              : $src),
             !strconcat(asm, " $dest"), pattern> {
  let Constraints = "$src = $dest";
}

class PseudoArithUnaryInstr<string asm, list<dag> pattern>
    : Pseudo<(outs GPRegs
              : $dest),
             (ins GPRegs
              : $src, uimm5
              : $imm),
             !strconcat(asm, " $dest, $imm"), pattern> {
  let Constraints = "$src = $dest";
}

class ShiftInstr<bits<5> opc, string asm, SDNode opNode, int amt = 1>
    : ArithUnaryInstr<opc, asm, [(set GPRegs
                                  : $dest, (opNode GPRegs
                                            : $src, amt))]> {
}

class PseudoShiftInstr<string asm, SDNode opNode>
    : PseudoArithUnaryInstr<asm, [(set GPRegs
                                   : $dest, (opNode GPRegs
                                             : $src, uimm5
                                             : $imm))]> {}

let Defs = [FLAGS] in {
  // ALU-esque instructions
  defm ADD : ArithInstr<0b0011, "add", add>;
  def PseudoADDri
      : Pseudo<(outs GPRegs
                : $dest),
               (ins GPRegs
                : $sdest, simm8
                : $imm),
               "!PseudoADDri $dest, $imm", [(set GPRegs
                                             : $dest, (add GPRegs
                                                       : $sdest, simm8
                                                       : $imm))]>;

  defm SUB : ArithInstr<0b0100, "sub", sub>;

  def CMPrr : RRI<0b1001, (outs),
                  (ins GPRegs
                   : $lhs, GPRegs
                   : $rhs, condcode
                   : $cc),
                  "cmp$cc $lhs, $rhs", [(sdma_cmp GPRegs
                                         : $lhs, GPRegs
                                         : $rhs, imm
                                         : $cc)]> {
    bits<2> cc;
    bits<3> lhs;
    bits<3> rhs;

    let Inst{15...11} = 0;
    let Inst{10...8} = lhs;
    let Inst{7...5} = 0b110;
    let Inst{4...3} = cc;
    let Inst{2...0} = rhs;
  }

  def CMPri : SDMAInstr<(outs),
                        (ins GPRegs
                         : $lhs, uimm8
                         : $rhs),
                        "cmpeq $lhs, $rhs", [(sdma_cmp GPRegs
                                              : $lhs, uimm8
                                              : $rhs, 0)]> {
    bits<3> rhs;
    bits<8> lhs;

    let Inst{15...11} = 0b01001;
    let Inst{10...8} = rhs;
    let Inst{7...0} = lhs;
  }
}

defm AND : ArithInstr<0b0111, "and", and>;
defm OR : ArithInstr<0b0101, "or", or>;
defm XOR : ArithInstr<0b0010, "xor", xor>;

def ANDNrr : RRI<0b0110,
                 (outs GPRegs
                  : $dest),
                 (ins GPRegs
                  : $sdest, GPRegs
                  : $src),
                 "andn $dest, $src", [(set GPRegs
                                       : $dest, (and GPRegs
                                                 : $sdest, (not GPRegs
                                                            : $src)))]> {
  let Constraints = "$sdest = $dest";
}

def ANDNri : RII<0b0110,
                 (outs GPRegs
                  : $dest),
                 (ins GPRegs
                  : $sdest, uimm8
                  : $imm),
                 "andn $dest, $imm", [(set GPRegs
                                       : $dest, (and GPRegs
                                                 : $sdest, (not uimm8
                                                            : $imm)))]> {
  let Constraints = "$sdest = $dest";
}

def ASR1 : ShiftInstr<0b10110, "asr1", sra>;
def LSL1 : ShiftInstr<0b10111, "lsl1", shl>;
def LSR1 : ShiftInstr<0b10101, "lsr1", srl>;
def PseudoASR : PseudoShiftInstr<"!PseudoASR", sra>;
def PseudoLSL : PseudoShiftInstr<"!PseudoLSL", shl>;
def PseudoLSR : PseudoShiftInstr<"!PseudoLSR", srl>;

def ROR1 : ShiftInstr<0b10100, "ror1", rotr>;
def REVB : ArithUnaryInstr<0b10000, "revb", [(set GPRegs
                                              : $dest, (bswap GPRegs
                                                        : $src))]>;
def REVBLO : ArithUnaryInstr<0b10001, "revblo", []>;
def RORB : ShiftInstr<0b10010, "rorb", rotr, 8>;

// control flow
let isTerminator = 1, isBarrier = 1, Uses = [PC] in {
  let isBranch = 1 in {
    let Uses = [FLAGS] in {
      def BDF : CondBrI<0b11, (outs),
                        (ins rel_brtarget
                         : $target),
                        "bdf $target", []>;
      def BSF : CondBrI<0b10, (outs),
                        (ins rel_brtarget
                         : $target),
                        "bsf $target", []>;
      def BF : CondBrI<0b00, (outs),
                       (ins rel_brtarget
                        : $target),
                       "bf $target", [(sdma_brf bb
                                       : $target)]>;
      def BT : CondBrI<0b01, (outs),
                       (ins rel_brtarget
                        : $target),
                       "bt $target", [(sdma_brt bb
                                       : $target)]>;
    }

    def JMP : BrI<0, (outs),
                  (ins abs_brtarget
                   : $target),
                  "jmp $target", [(br bb
                                   : $target)]>;

    let isIndirectBranch = 1, Uses = [PC] in {
      def JMPR : SDMAInstr<(outs),
                           (ins GPRegs
                            : $target),
                           "jmpr $target", [(brind GPRegs
                                             : $target)]> {
        bits<3> target;
        let Inst{15...11} = 0;
        let Inst{8...10} = target;
        let Inst{7...0} = 0b00001000;
      }
    }
  }

  def RET : SDMAInstr<(outs), (ins), "ret", [(ret)]> {
    let isReturn = 1;
    let Uses = [RPC];
    let Defs = [PC];
    let Inst = 0b0000000000000110;
  }
}

let isCall = 1, Defs = [RPC, PC] in {
  def JSR : BrI<1, (outs),
                (ins uimm14
                 : $target),
                "jsr $target", [(call tglobaladdr
                                 : $target)]>;
  def JSRR : UnaryI<0b01001, (outs),
                    (ins GPRegs
                     : $dest),
                    "jsrr $dest", [(call GPRegs
                                    : $dest)]>;
}

// Data manip
let isMoveImm = 1 in {
  def LDI : RII<0b0001,
                (outs GPRegs
                 : $dest),
                (ins uimm8
                 : $imm),
                "ldi $dest, $imm", [(set GPRegs
                                     : $dest, uimm8
                                     : $imm)]>;

  def PseudoLoadLargeU
      : Pseudo<(outs GPRegs
                : $dest),
               (ins uimm8
                : $imm),
               "!PseudoLoadLargeU $dest, $imm", [(set GPRegs
                                                  : $dest, uimm32
                                                  : $imm)]>;
  def PseudoLoadLargeS
      : Pseudo<(outs GPRegs
                : $dest),
               (ins uimm8
                : $imm),
               "!PseudoLoadLargeS $dest, $imm", [(set GPRegs
                                                  : $dest, simm32
                                                  : $imm)]>;
}

let isMoveReg = 1 in {
  def MOV : RRI<0b0001,
                (outs GPRegs
                 : $dest),
                (ins GPRegs
                 : $src),
                "mov $dest, $src", [(set GPRegs
                                     : $dest, GPRegs
                                     : $src)]>;

  def LDRPC : UnaryI<0b01010,
                     (outs GPRegs
                      : $dest),
                     (ins SpecialRegs
                      : $src),
                     "ldrpc $dest", [(set GPRegs
                                      : $dest, SpecialRegs
                                      : $src)]> {
    let Uses = [RPC];
  }
}

// order: [ADDR] = Value
def ST : MemI<1, (outs),
              (ins(MEMri $base, $offset)
               : $addr, GPRegs
               : $value),
              "st $value, [$addr]", [(store GPRegs
                                      : $value, ADDRri
                                      : $addr)]>;

// order: value = [ADDR]
def LD : MemI<0,
              (outs GPRegs
               : $value),
              (ins(MEMri $base, $offset)
               : $addr),
              "ld $value, [$addr]", [(set GPRegs
                                      : $value, (load ADDRri
                                                 : $addr))]>;
// stack
def ADJCALLSTACKDOWN
    : Pseudo<(outs),
             (ins i32imm
              : $amt1, i32imm
              : $amt2),
             "!ADJCALLSTACKDOWN $amt1, $amt2", [(callseq_start timm
                                                 : $amt1, timm
                                                 : $amt2)]>;

def ADJCALLSTACKUP : Pseudo<(outs),
                            (ins i32imm
                             : $amt1, i32imm
                             : $amt2),
                            "!ADJCALLSTACKUP $amt1", [(callseq_end timm
                                                       : $amt1, timm
                                                       : $amt2)]>;

// non-selected instructions
def STF : MemFunctionalI<1, (outs),
                         (ins uimm8
                          : $base, GPRegs
                          : $value),
                         "stf $value, [$base]", []>;

def LDF : MemFunctionalI<0,
                         (outs GPRegs
                          : $value),
                         (ins uimm8
                          : $base),
                         "ldf $value, [$base]", []>;

def HWLOOP : SDMAInstr<(outs),
                       (ins uimm8
                        : $n, uimm2
                        : $ff),
                       "loop $n, $ff", []> {
  bits<8> n;
  bits<2> ff;

  let Inst{15...10} = 0b011110;
  let Inst{9...8} = ff;
  let Inst{7...0} = n;
}

class ControlI<string asm, bit opc> : SDMAInstr<(outs),
                                                (ins uimm3
                                                 : $j),
                                                !strconcat(asm, " $j"), []> {
  bits<3> j;

  let Inst{15...11} = 0;
  let Inst{10...8} = j;
  let Inst{7...1} = 0;
  let Inst{0} = opc;
}

def DONE : ControlI<"done", 0>;
def NOTIFY : ControlI<"notify", 1>;

class BitManipInstr<bits<2> opc, string asm, list<dag> pattern>
    : BitManipI<opc,
                (outs GPRegs
                 : $dest),
                (ins GPRegs
                 : $sdest, uimm8
                 : $index),
                !strconcat(asm, " $dest, $index"), pattern> {
  let Constraints = "$sdest = $dest";
}

def BCLR : BitManipInstr<0b01, "bclr", []>;
def BSET : BitManipInstr<0b10, "bset", []>;
def BTST : BitManipInstr<0b11, "btst", []>;

// TODO: select bit-test
def TST : RII<0b1000, (outs),
              (ins GPRegs
               : $dest, uimm8
               : $imm),
              "tst $dest, $imm", []>;
