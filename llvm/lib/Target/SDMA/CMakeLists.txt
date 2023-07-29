add_llvm_component_group(SDMA)

set(LLVM_TARGET_DEFINITIONS SDMA.td)

tablegen(LLVM SDMAGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM SDMAGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM SDMAGenCallingConv.inc -gen-callingconv)
tablegen(LLVM SDMAGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM SDMAGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM SDMAGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM SDMAGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM SDMAGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM SDMAGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(SDMACommonTableGen)

add_llvm_target(SDMACodeGen
    SDMAAsmPrinter.cpp
    SDMAInstrInfo.cpp
    SDMAISelDAGToDAG.cpp
    SDMAISelLowering.cpp
    SDMAFrameLowering.cpp
    SDMAMCInstLower.cpp
    SDMASubtarget.cpp
    SDMARegisterInfo.cpp
    SDMATargetMachine.cpp
    SDMAInstrInfo.cpp
    SDMAMachineFunctionInfo.cpp
    SDMAISelDAGToDAG.cpp
    SDMATargetObjectFile.cpp
    SDMAExpandPseudoInsts.cpp

    LINK_COMPONENTS
    AsmPrinter
    CodeGen
    CodeGenTypes
    Core
    MC
    SelectionDAG
    SDMADesc
    SDMAInfo
    Support
    Target
    TargetParser

    ADD_TO_COMPONENT
    SDMA
)

#add_subdirectory(AsmParser)
#add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
##add_subdirectory(TargetInfo)
add_subdirectory(TargetInfo)
