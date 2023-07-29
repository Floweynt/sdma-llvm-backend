include "llvm/Target/Target.td"

include "SDMARegisterInfo.td"
include "SDMACallingConv.td"
include "SDMAInstrInfo.td"

def SDMAInstrInfo : InstrInfo;

def : Processor<"generic", NoItineraries, []>;

def SDMA : Target {
  let InstructionSet = SDMAInstrInfo;
}
