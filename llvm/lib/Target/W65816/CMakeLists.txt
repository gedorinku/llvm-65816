add_llvm_component_group(W65816)

set(LLVM_TARGET_DEFINITIONS W65816.td)

tablegen(LLVM W65816GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM W65816GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM W65816GenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM W65816GenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM W65816GenCallingConv.inc -gen-callingconv)

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
