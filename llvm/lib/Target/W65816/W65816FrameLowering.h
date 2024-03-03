#ifndef LLVM_LIB_TARGET_W65816_W65816FRAMELOWERING_H
#define LLVM_LIB_TARGET_W65816_W65816FRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class W65816FrameLowering : public TargetFrameLowering {
  explicit W65816FrameLowering()
      : TargetFrameLowering(StackGrowsDown,
                            /*StackAlignment=*/Align(16),
                            /*LocalAreaOffset=*/0) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_W65816_W65816FRAMELOWERING_H
