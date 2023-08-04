class SDMAReg<string name> : Register<name> {
  let Namespace = "sdma";
}

class GPReg<string name, bits<3> num> : SDMAReg<name> {
  bits<3> Num = num;
}

class SpecialReg<string name> : SDMAReg<name> {
}

def GP0 : GPReg<"GP0", 0>;
def GP1 : GPReg<"GP1", 1>;
def GP2 : GPReg<"GP2", 2>;
def GP3 : GPReg<"GP3", 3>;
def GP4 : GPReg<"GP4", 4>;
def GP5 : GPReg<"GP5", 5>;

def GP6 : GPReg<"GP6", 6>; // we pretend that this is a frame pointer
def GP7 : GPReg<"GP7", 7>; // we pretend that this is a stack pointer

def PC : SpecialReg<"PC">;
def RPC : SpecialReg<"RPC">;
def FLAGS : SpecialReg<"flags">;

def GPRegs : RegisterClass<"sdma", [i32], 32, (sequence "GP%u", 0, 7)>;
def SpecialRegs : RegisterClass<"sdma", [i32], 32, (add PC, RPC, FLAGS)>;

