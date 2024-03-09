#include "W65816RegisterInfo.h"
#include "W65816.h"
#include "W65816FrameLowering.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include <iterator>

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "W65816GenRegisterInfo.inc"

W65816RegisterInfo::W65816RegisterInfo(const W65816Subtarget &Subtarget,
                                       unsigned HwMode)
    : W65816GenRegisterInfo(0, 0, 0, 0, HwMode), Subtarget(Subtarget) {}

const MCPhysReg *
W65816RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_STACK_SaveList;
}

const uint32_t *
W65816RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                         CallingConv::ID) const {
  return CSR_STACK_RegMask;
}

BitVector W65816RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  static const uint16_t ReservedCPURegs[] = {W65816::D, W65816::SP};
  BitVector Reserved(getNumRegs());

  for (unsigned I = 0; I < std::size(ReservedCPURegs); ++I)
    Reserved.set(ReservedCPURegs[I]);

  return Reserved;
}

bool W65816RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                             int SPAdj,
                                             unsigned int FIOperandNum,
                                             RegScavenger *RS) const {
  // todo
  return true;
}

Register W65816RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
  return TFI->hasFP(MF) ? (W65816::D) : (W65816::SP);
}
