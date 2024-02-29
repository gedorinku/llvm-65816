add_llvm_component_group(W65816)

add_llvm_target(W65816CodeGen
  W65816TargetMachine.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  IPO
  MC
  W65816Info
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils

  ADD_TO_COMPONENT
  W65816
  )

add_subdirectory(TargetInfo)
