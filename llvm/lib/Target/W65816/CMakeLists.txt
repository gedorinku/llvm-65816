add_llvm_component_group(W65816)

set(LLVM_TARGET_DEFINITIONS W65816.td)

tablegen(LLVM W65816GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM W65816GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM W65816GenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM W65816GenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM W65816GenCallingConv.inc -gen-callingconv)

tablegen(LLVM W65816GenAsmWriter.inc -gen-asm-writer)

add_public_tablegen_target(W65816CommonTableGen)

add_llvm_target(W65816CodeGen
  W65816AsmPrinter.cpp
  W65816FrameLowering.cpp
  W65816InstrInfo.cpp
  W65816ISelLowering.cpp
  W65816RegisterInfo.cpp
  W65816Subtarget.cpp
  W65816TargetMachine.cpp
  W65816TargetObjectFile.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  IPO
  MC
  W65816Desc
  W65816Info
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils

  ADD_TO_COMPONENT
  W65816
  )

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
