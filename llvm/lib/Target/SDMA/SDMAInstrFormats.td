class SDMAInstr<dag outOper, dag inOper, string asm, list<dag> pattern>
    : Instruction {
  let Namespace = "sdma";
  let OutOperandList = outOper;
  let InOperandList = inOper;
  let AsmString = asm;
  let Pattern = pattern;
  bits<16> Inst = 0;
}

class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
    : SDMAInstr<outs, ins, asmstr, pattern> {
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

/*
Encodes:
00000xxx10000xxx - reserved
00000rrr10001sss - mov
00000rrr10010sss - xor
00000rrr10011sss - add
00000rrr10100sss - sub
00000rrr10101sss - or
00000rrr10110sss - andn
00000rrr10111sss - and
00000rrr11000sss - tst
00000rrr11001sss - cmpeq
00000rrr11010sss - cmplt
00000rrr11011sss - cmphs
*/
class RRI<bits<4> opc, dag outOper, dag inOper, string asm, list<dag> pattern>
    : SDMAInstr<outOper, inOper, asm, pattern> {
  bits<3> src;
  bits<3> dest;

  let Inst{0...2} = src;
  let Inst{3...6} = opc;
  let Inst{7} = 1;
  let Inst{8...10} = dest;
  let Inst{11...15} = 0b00000;
}

/*
Encodes:
00001rrriiiiiiii - ldi r,i
00010rrriiiiiiii - xori r,i
00011rrriiiiiiii - addi r,i
00100rrriiiiiiii - subi r,i
00101rrriiiiiiii - ori r,i
00110rrriiiiiiii - andni r,i
00111rrriiiiiiii - andi r,i
01000rrriiiiiiii - tsti r,i
01001rrriiiiiiii - cmpeqi r,i
01100rrruuuuuuuu - ldf r,u
01101rrruuuuuuuu - stf r,u
*/
class RII<bits<4> opc, dag outOper, dag inOper, string asm, list<dag> pattern>
    : SDMAInstr<outOper, inOper, asm, pattern> {
  bits<3> dest;
  bits<8> imm;

  let Inst{0...7} = imm;
  let Inst{8...10} = dest;
  let Inst{11...14} = opc;
  let Inst{15} = 0;
}

/*
Encodes:
00000rrr00001000 - jmpr r
00000rrr00001001 - jsrr
00000rrr00001010 - ldrpc r
00000rrr00001011 - reserved
00000rrr000011xx - reserved
00000rrr00010000 - revb
00000rrr00010001 - revblo
00000rrr00010010 - rorb
00000rrr00010011 - reserved
00000rrr00010100 - ror1
00000rrr00010101 - lsr1
00000rrr00010110 - asr1
00000rrr00010111 - lsl1
*/
class UnaryI<bits<5> opc, dag outOper, dag inOper, string asm,
             list<dag> pattern> : SDMAInstr<outOper, inOper, asm, pattern> {
  bits<3> dest;

  let Inst{0...4} = opc;
  let Inst{5...7} = 0;
  let Inst{8...10} = dest;
  let Inst{11...15} = 0b00000;
}

/*
00000rrr001nnnnn - bclri r,n
00000rrr010nnnnn - bseti r,n
00000rrr011nnnnn - btsti r,n
*/
class BitI<bits<2> opc, dag outOper, dag inOper, string asm, list<dag> pattern>
    : SDMAInstr<outOper, inOper, asm, pattern> {
  bits<3> reg;
  bits<5> imm;

  let Inst{0...4} = imm;
  let Inst{5...6} = opc;
  let Inst{7} = 0;
  let Inst{8...10} = reg;
  let Inst{11...15} = 0b00000;
}

/*
01010rrrdddddbbb - ld r,(d,b)
01011rrrdddddbbb - st r,u
*/

class MemI<bit store, dag outOper, dag inOper, string asm, list<dag> pattern>
    : SDMAInstr<outOper, inOper, asm, pattern> {
  bits<3> value;
  bits<3> base;
  bits<5> offset;

  let Inst{0...2} = base;
  let Inst{3...7} = offset;
  let Inst{8...10} = value;
  let Inst{11} = store;
  let Inst{12...15} = 0b0101;
}

/*
01111100pppppppp - bf pc=pc+signed(pppppppp)+1
01111101pppppppp - bt pc=pc+signed(pppppppp)+1
01111110pppppppp - bsf pc=pc+signed(pppppppp)+1
01111111pppppppp - bdf pc=pc+signed(pppppppp)+1
*/

class CondBrI<bits<2> opc, dag outOper, dag inOper, string asm,
              list<dag> pattern> : SDMAInstr<outOper, inOper, asm, pattern> {
  bits<8> target;

  let Inst{0...7} = target;
  let Inst{8...9} = opc;
  let Inst{10...15} = 0b011111;
}

/*
10aaaaaaaaaaaaaa - jmp absolute
11aaaaaaaaaaaaaa - jsr absolute
*/
class BrI<bit opc, dag outOper, dag inOper, string asm, list<dag> pattern>
    : SDMAInstr<outOper, inOper, asm, pattern> {
  bits<14> target;

  let Inst{0...13} = target;
  let Inst{14} = opc;
  let Inst{15} = 1;
}
