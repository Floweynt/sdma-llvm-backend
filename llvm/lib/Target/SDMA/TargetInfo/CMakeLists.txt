add_llvm_component_library(LLVMSDMAInfo
  SDMATargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  SDMA
  )
