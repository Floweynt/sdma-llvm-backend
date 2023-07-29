def CC_SDMA : CallingConv<[
    CCIfType<[i32], CCAssignToReg<[GP0, GP1, GP2, GP3]>>,
    CCAssignToStack<4, 4>
]>;

def RetCC_SDMA : CallingConv<[
  // we just use gp0
  CCIfType<[i32], CCAssignToReg<[GP0]>>,
]>;

// Callee-saved register lists

// The caller must save SPC EPC for performance reasons (hardware loops are an uncommon pattern)
// The caller must save GP0 because it is the return value
// The caller must save GP1-GP3 because they are arguments
// The caller must save RPC since jsr(r) overwrites it

// That leaves GP4-7 must be callee-save
def CSR : CalleeSavedRegs<(add RPC, (sequence "GP%u", 4, 7))>;
