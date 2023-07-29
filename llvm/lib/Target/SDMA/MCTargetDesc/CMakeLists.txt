add_llvm_component_library(LLVMSDMADesc
  SDMAMCTargetDesc.cpp
  SDMAInstPrinter.cpp
  SDMAMCExpr.cpp

  LINK_COMPONENTS
  MC
  SDMAInfo
  Support
  TargetParser

  ADD_TO_COMPONENT
  SDMA
  )
