#ifndef LLVM_LIB_TARGET_W65816_W65816REGISTERINFO_H
#define LLVM_LIB_TARGET_W65816_W65816REGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "W65816GenRegisterInfo.inc"

namespace llvm {

class W65816RegisterInfo : public W65816GenRegisterInfo {
  W65816RegisterInfo();

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;
  bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_W65816_W65816REGISTERINFO_H
