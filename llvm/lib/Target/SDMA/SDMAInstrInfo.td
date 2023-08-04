// SelectionDAG stuff
def ret : SDNode<"SDMAISD::RET", SDTNone,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def call : SDNode<"SDMAISD::CALL", SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDCallSeqStart<[ SDTCisVT<0, i32>,
                                          SDTCisVT<1, i32> ]>,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDCallSeqEnd<[ SDTCisVT<0, i32>,
                                        SDTCisVT<1, i32> ]>,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def sdma_cmp : SDNode<"SDMAISD::CMP", SDTypeProfile<0, 3, [SDTCisSameAs<0, 1>, SDTCisVT<2, i32>]>, [SDNPOutGlue]>;
def sdma_brt : SDNode<"SDMAISD::BR_T", SDTypeProfile<0, 1, [SDTCisVT<0, OtherVT>]>, [SDNPHasChain, SDNPInGlue]>;
def sdma_brf : SDNode<"SDMAISD::BR_F", SDTypeProfile<0, 1, [SDTCisVT<0, OtherVT>]>, [SDNPHasChain, SDNPInGlue]>;

// Operands
class ImmAsmOperand<string prefix, int width, string suffix = ""> : AsmOperandClass {
  let Name = prefix # "Imm" # width # suffix;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid ", Name);
}

class uimm<int width> : Operand<i32>, ImmLeaf<i32, !strconcat("return isUInt<", !cast<string>(width), ">(Imm);")> {
  let ParserMatchClass = ImmAsmOperand<"U", width>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = !strconcat("decodeUImmOperand<", !cast<string>(width), ">");
  let MCOperandPredicate = !strconcat([{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isUInt<}], !cast<string>(width), [{>(Imm);
    return MCOp.isBareSymbolRef();
  }]);
  let OperandType = !strconcat("OPERAND_UIMM", !cast<string>(width));
  let OperandNamespace = "sdma";
}

class simm<int width> : Operand<i32>, ImmLeaf<i32, !strconcat("return isInt<", !cast<string>(width), ">(Imm);")> {
  let ParserMatchClass = ImmAsmOperand<"S", width>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = !strconcat("decodeSImmOperand<", !cast<string>(width), ">");
  let MCOperandPredicate = !strconcat([{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isSInt<}], !cast<string>(width), [{>(Imm);
    return MCOp.isBareSymbolRef();
  }]);
  let OperandType = !strconcat("OPERAND_UIMM", !cast<string>(width));
  let OperandNamespace = "sdma";
}

def uimm5 : uimm<5>;
def uimm8 : uimm<5>;
def uimm14 : uimm<14>;
def uimm32 : uimm<32>;

def simm8 : simm<8>;
def simm32 : simm<32>;

def SDMAMEMrAsmOperand : AsmOperandClass {
  let Name = "MEMr";
  let ParserMethod = "parseMEMOperand";
}

def SDMAMEMriAsmOperand : AsmOperandClass {
  let Name = "MEMri";
  let ParserMethod = "parseMEMOperand";
}

def MEMri : Operand<iPTR> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops ptr_rc, i32imm);
  let ParserMatchClass = SDMAMEMriAsmOperand;
}

def ADDRri : ComplexPattern<iPTR, 2, "SelectADDRri", [frameindex], []>;

def brtarget : Operand<OtherVT> {
  let OperandType = "OPERAND_BRTARGET";
  let OperandNamespace = "sdma";
  let PrintMethod = "printBranchTarget";
  let EncoderMethod = "encodeBranchTarget";
}

def condcode : Operand<i32> {
  let OperandType = "OPERAND_CONDCODE";
  let OperandNamespace = "sdma";
  let PrintMethod = "printCondType";
  let EncoderMethod = "encodeCondType";
}

class SDMAInstr<dag outOper, dag inOper, string asm, list<dag> pattern, bits<16> inst> : Instruction {
  let Namespace = "sdma";
  let OutOperandList = outOper;
  let InOperandList = inOper;
  let AsmString = asm;
  let Pattern = pattern;
  bits<16> Inst = inst; // TODO: implement
}

class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
   : SDMAInstr<outs, ins, asmstr, pattern, 0> {
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

multiclass ArithInstr<string opc, SDNode opNode> {
  def rr : SDMAInstr<
    (outs GPRegs:$dest), 
    (ins GPRegs:$sdest, GPRegs:$src),
    !strconcat(opc, " $dest, $src ; $dest = ", opc, "($dest, $src)"),
    [(set GPRegs:$dest, (opNode GPRegs:$sdest, GPRegs:$src))],
    0 // TODO: fix
  > {
    let Constraints = "$sdest = $dest";
  }
  def ri : SDMAInstr<
    (outs GPRegs:$dest),
    (ins GPRegs:$sdest, uimm8:$imm),
    !strconcat(opc, "i $dest, $imm ; $dest = ", opc, "($dest, $imm)"),
    [(set GPRegs:$dest, (opNode GPRegs:$sdest, uimm8:$imm))],
    0 // TODO: fix
  > {
    let Constraints = "$sdest = $dest";
  }
}

multiclass ArithInstrExt<string opc, dag op> {
  def rr : SDMAInstr<
    (outs GPRegs:$dest), 
    (ins GPRegs:$sdest, GPRegs:$src),
    !strconcat(opc, " $dest, $src ; $dest = ", opc, "($dest, $src)"),
    [(set GPRegs:$dest, op)],
    0 // TODO: fix
  > { 
    let Constraints = "$sdest = $dest";
  }
  /*
  def ri : SDMAInstr<
    (outs GPRegs:$dest),
    (ins GPRegs:$sdest, uimm8:$imm),
    !strconcat(opc, "i $dest, $imm ; $dest = ", opc, "($dest, $imm)"),
    [(set GPRegs:$dest, (opNode GPRegs:$sdest, uimm8:$imm))],
    "$sdest = $dest",
    0 // TODO: fix
  >;*/
}


/* 
TODO instructions

BCLRI
BSETI
BTSTI
CLRF
cpShReg
DONE
ILLEGAL
JMP
JMPR
LDF
LDRPC
LOOP
NOTIFY
REVBLO
SOFTBKPT
ST
STF
TST
TSTI
*/

class ArithUnaryInstr<string opc, list<dag> pattern> : SDMAInstr<(outs GPRegs:$dest), (ins GPRegs:$src), 
  !strconcat(opc, " $dest ; $dest = ", opc, "($src)"), pattern, 0> {
  let Constraints = "$src = $dest";
}

class PseudoArithUnaryInstr<string opc, list<dag> pattern> : Pseudo<(outs GPRegs:$dest), (ins GPRegs:$src, uimm5:$imm), 
  !strconcat(opc, " $dest, $imm"), pattern> {
  let Constraints = "$src = $dest";
}

class ShiftInstr<string opc, SDNode opNode, int amt = 1> : ArithUnaryInstr<opc, [(set GPRegs:$dest, (opNode GPRegs:$src, amt))]> 
{
}

class PseudoShiftInstr<string opc, SDNode opNode> : PseudoArithUnaryInstr<opc, [(set GPRegs:$dest, (opNode GPRegs:$src, uimm5:$imm))]> 
{
}

let Defs = [FLAGS] in {
  // ALU-esque instructions
  defm ADD : ArithInstr<"add", add>;
  // handle LLVM stupidness
  def PseudoADDri : Pseudo<(outs GPRegs:$dest),  (ins GPRegs:$sdest, simm8:$imm), "!PseudoADDri $dest, $imm", 
    [(set GPRegs:$dest, (add GPRegs:$sdest, simm8:$imm))]>;
  defm SUB : ArithInstr<"sub", sub>;
  def CMPrr : SDMAInstr<(outs), (ins GPRegs:$rhs, GPRegs:$lhs, condcode:$cc), "cmp$cc $rhs, $lhs ; flag = $rhs == $lhs", [(sdma_cmp GPRegs:$rhs, GPRegs:$lhs, imm:$cc)], 0>;
  def CMPri : SDMAInstr<(outs), (ins GPRegs:$rhs, uimm8:$lhs, condcode:$cc), "cmp$cc $rhs, $lhs ; flag = $rhs == $lhs", [(sdma_cmp GPRegs:$rhs, uimm8:$lhs, imm:$cc)], 0>;
}

defm AND : ArithInstr<"and", and>;
defm OR : ArithInstr<"or", or>;
defm XOR : ArithInstr<"xor", xor>;
defm ANDN : ArithInstrExt<"andn", (and GPRegs:$sdest, (not GPRegs:$src))>;

def ASR1 : ShiftInstr<"asr1", sra>;
def LSL1 : ShiftInstr<"lsl1", shl>;
def LSR1 : ShiftInstr<"lsr1", srl>;
def PseudoASR : PseudoShiftInstr<"!PseudoASR", sra>;
def PseudoLSL : PseudoShiftInstr<"!PseudoLSL", shl>;
def PseudoLSR : PseudoShiftInstr<"!PseudoLSR", srl>;

def ROR1 : ShiftInstr<"ror1", rotr>;
def REVB : ArithUnaryInstr<"revb", [(set GPRegs:$dest, (bswap GPRegs:$src))]>;
def REVBLO : ArithUnaryInstr<"revblo", []>;
def RORB : ShiftInstr<"rorb", rotr, 8>;

// control flow
let isTerminator = 1, isBarrier = 1 in {
  let isBranch = 1 in {
    let Uses = [FLAGS] in {
      def BDF : SDMAInstr<(outs), (ins brtarget:$target), "bdf $target", [], 0>;
      def BSF : SDMAInstr<(outs), (ins brtarget:$target), "bsf $target", [], 0>;
      def BF : SDMAInstr<(outs), (ins brtarget:$target), "bf $target", [(sdma_brf bb:$target)], 0>;
      def BT : SDMAInstr<(outs), (ins brtarget:$target), "bt $target", [(sdma_brt bb:$target)], 0>;
    }

    def JMP : SDMAInstr<(outs), (ins brtarget:$target), "jmp $target ; %pc = $target", [(br bb:$target)], 0>;
    //def JMP : SDMAInstr<(outs), (ins), "jmp", [], 0>;
  }

  def RET : SDMAInstr<(outs), (ins), "ret ; %pc = %rpc", [(ret)], 0b0000000000000110> {
    let isReturn = 1;
  }
}

let isCall = 1, Defs = [RPC, PC] in {
  def JSR : SDMAInstr<(outs), (ins uimm14:$target), "jsr $target ; %pc = &$target", [(call tglobaladdr:$target)], 0>;
  def JSRR : SDMAInstr<(outs), (ins GPRegs:$reg), "jsrr $reg ; %pc = $reg[0:14]", [(call GPRegs:$reg)], 0>;
}

// Data manip
let isMoveImm = 1 in {
  def LDI : SDMAInstr<(outs GPRegs:$dest), (ins uimm8:$imm), "ldi $dest, $imm ; $dest = $imm", [(set GPRegs:$dest, uimm8:$imm)], 0>;
  def PseudoLoadLargeU : Pseudo<(outs GPRegs:$dest), (ins uimm8:$imm), "!PseudoLoadLargeU $dest, $imm ; $dest = $imm", [(set GPRegs:$dest, uimm32:$imm)]>;
  def PseudoLoadLargeS : Pseudo<(outs GPRegs:$dest), (ins uimm8:$imm), "!PseudoLoadLargeS $dest, $imm ; $dest = $imm", [(set GPRegs:$dest, simm32:$imm)]>;
}

let isMoveReg = 1 in {
  def MOV : SDMAInstr<(outs GPRegs:$dest), (ins GPRegs:$src), "mov $dest, $src ; $dest = $src", [(set GPRegs:$dest, GPRegs:$src)], 0>;
  def LDRPC : SDMAInstr<(outs GPRegs:$dest), (ins SpecialRegs:$src), "ldrpc $dest ; $dest = RPC", [(set GPRegs:$dest, SpecialRegs:$src)], 0>;
}

// order: [ADDR] = Value
def ST : SDMAInstr<
  (outs), 
  (ins (MEMri $base, $uimm5):$addr, GPRegs:$value), 
  "st [$addr], $value ; *($addr) = $value", 
  [(store GPRegs:$value, ADDRri:$addr)], 
  0
>;

// order: value = [ADDR]
def LD : SDMAInstr<
  (outs GPRegs:$value), 
  (ins (MEMri $base, $uimm5):$addr), 
  "ld [$addr], $value ; *($addr) = $value", 
  [(set GPRegs:$value, (load ADDRri:$addr))], 
  0
>;

def LD_off : SDMAInstr<(outs GPRegs:$value), (ins GPRegs:$base), "ld $base, 0, $value ; *($base) = $value", [(set GPRegs:$value, (load GPRegs:$base))], 0>;

// weird stuff
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                               "!ADJCALLSTACKDOWN $amt1, $amt2",
                               [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                            "!ADJCALLSTACKUP $amt1",
                            [(callseq_end timm:$amt1, timm:$amt2)]>;

