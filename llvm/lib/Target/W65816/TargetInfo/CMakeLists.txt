add_llvm_component_library(LLVMW65816Info
  W65816TargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  W65816
)
