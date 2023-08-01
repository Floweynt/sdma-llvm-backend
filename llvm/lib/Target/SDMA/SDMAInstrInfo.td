// SelectionDAG stuff
def ret : SDNode<"SDMAISD::RET", SDTNone,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def call : SDNode<"SDMAISD::CALL", SDTypeProfile<0, 1, [SDTCisVT<0, i32>]>,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDCallSeqStart<[ SDTCisVT<0, i32>,
                                          SDTCisVT<1, i32> ]>,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDCallSeqEnd<[ SDTCisVT<0, i32>,
                                        SDTCisVT<1, i32> ]>,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def sdma_cmp     : SDNode<"SDMAISD::CMP", SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>]>, [SDNPOutGlue]>;
def sdma_brcc    : SDNode<"SDMAISD::BR_CC", SDTypeProfile<0, 2, [SDTCisVT<0, OtherVT>,
                              SDTCisVT<1, i8>]>,
                            [SDNPHasChain, SDNPInGlue]>;

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

class brtarget<ValueType ty> : Operand<ty> {
  let PrintMethod = "printPCRelImm";
  let EncoderMethod = "encodeBrTarget";
}

def uimm5 : uimm<5>;
def uimm8 : uimm<5>;
def uimm14 : uimm<14>;
def uimm32 : uimm<32>;

def simm8 : simm<8>;
def simm32 : simm<32>;

def brtarget8 : brtarget<OtherVT>;

def condcode : Operand<OtherVT> {
  let PrintMethod = "printPCRelImm";
  let EncoderMethod = "encodeBrTarget";
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
CMPEQ
CMPEQI
CMPHS
CMPLT
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

// ALU-esque instructions
defm ADD : ArithInstr<"add", add>;
// handle LLVM stupidness
def PseudoADDri : Pseudo<(outs GPRegs:$dest),  (ins GPRegs:$sdest, simm8:$imm), "!PseudoADDri $dest, $imm", 
  [(set GPRegs:$dest, (add GPRegs:$sdest, simm8:$imm))]>;
defm SUB : ArithInstr<"sub", sub>;
defm AND : ArithInstr<"and", and>;
defm OR : ArithInstr<"or", or>;
defm XOR : ArithInstr<"xor", xor>;
defm ANDN : ArithInstrExt<"andn", (and GPRegs:$sdest, (not GPRegs:$src))>;

def CMPEQrr : SDMAInstr<(outs), (ins GPRegs:$rhs, GPRegs:$lhs), "cmpeq $rhs, $lhs ; flag = $rhs == $lhs", [], 0>;
def CMPEQri : SDMAInstr<(outs), (ins GPRegs:$rhs, uimm8:$lhs), "cmpeq $rhs, $lhs ; flag = $rhs == $lhs", [], 0>;

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
    def BDF : SDMAInstr<(outs), (ins brtarget8:$target), "bdf $target", [], 0>;
    def BSF : SDMAInstr<(outs), (ins brtarget8:$target), "bsf $target", [], 0>;
    def BF : SDMAInstr<(outs), (ins brtarget8:$target), "bf $target", [], 0>;
    def BT : SDMAInstr<(outs), (ins brtarget8:$target), "bt $target", [], 0>;
    def JMP : SDMAInstr<(outs), (ins brtarget8:$target), "jmp $target ; %pc = $target", [(br bb:$target)], 0>;
    //def JMP : SDMAInstr<(outs), (ins), "jmp", [], 0>;
  }

  def RET : SDMAInstr<(outs), (ins), "ret ; %pc = %rpc", [(ret)], 0b0000000000000110> {
    let isReturn = 1;
  }
}

let isCall = 1 in {
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
// Data manip - memory
def ST : SDMAInstr<(outs), (ins GPRegs:$base, GPRegs:$value), "st $base, 0, $value ; *($base) = $value", [(store GPRegs:$value, GPRegs:$base)], 0>;
def ST_off : SDMAInstr<(outs), 
  (ins GPRegs:$base, GPRegs:$value, uimm5:$offset), 
  "st $base, $offset, $value ; *($base + $offset) = $value", [(store GPRegs:$value, (add GPRegs:$base, uimm5:$offset))], 0>;

def PseudoPUSH : Pseudo<(outs), (ins GPRegs:$reg), "!PseudoPUSH $reg", []>;
def PseudoPOP : Pseudo<(outs), (ins GPRegs:$reg), "!PseudoPOP $reg", []>;

// weird stuff
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                               "!ADJCALLSTACKDOWN $amt1, $amt2",
                               [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                            "!ADJCALLSTACKUP $amt1",
                            [(callseq_end timm:$amt1, timm:$amt2)]>;

// def Addrr : SDMAInstr<
//     (
// >;

// defm Add : ArithInstr<"add", add>;
// defm And : ArithInstr<"and", and>;
// defm Andn : ArithInstrExt<"andn", and, not>;

// def AShr1 : ArithUnaryInstr<(sra $rd, 1)>; // TODO: figure out how to specify imm of one 
// def Bclri : ArithUnaryInstr<(sra $rd, 1)>; // TODO: figure out dag for bitclear
// def Bdf TODO: implement
// def Bf 
// def Bseti : ArithUnaryInstr<(sra $rd, 1)>; // TODO: figure out dag for bitset
// def Bsf TODO: implement
// def Bt 
// def Btsti : ArithUnaryInstr<(sra $rd, 1)>; // TODO: figure out dag for bittest
// def Clrf 
// def Cmpeq : // idk
// def Cmpeqi : // IDK!
// def Cmphs
// def Cmplt
// def Jmp
// def Jmpr
// def Jsr
// def Jsrr
// def Ld
// def Ldf
// def Ldi
// def Ldrpc
