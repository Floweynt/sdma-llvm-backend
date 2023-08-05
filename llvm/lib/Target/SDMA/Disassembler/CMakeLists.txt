add_llvm_component_library(LLVMSDMADisassembler
  SDMADisassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  MC
  SDMAInfo
  Support

  ADD_TO_COMPONENT
  SDMA
  )
