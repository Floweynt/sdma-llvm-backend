include "llvm/Target/Target.td"

include "SDMARegisterInfo.td"
include "SDMACallingConv.td"
include "SDMAInstrInfo.td"

def SDMAInstrInfo : InstrInfo;

def Generic : Processor<"generic", NoItineraries, []>;

def SDMAInstPrinter : AsmWriter {
  string AsmWriterClassName = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def SDMA : Target {
  let InstructionSet = SDMAInstrInfo;
  let AssemblyWriters = [SDMAInstPrinter];
}
