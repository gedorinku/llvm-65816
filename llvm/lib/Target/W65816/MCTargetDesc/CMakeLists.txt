add_llvm_component_library(LLVMW65816Desc
  W65816ABIInfo.cpp
  W65816MCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  W65816Info
  Support
  TargetParser

  ADD_TO_COMPONENT
  W65816
)
