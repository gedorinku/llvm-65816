#ifndef LLVM_LIB_TARGET_W65816_W65816INSTRINFO_H
#define LLVM_LIB_TARGET_W65816_W65816INSTRINFO_H

#include "W65816RegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "W65816GenInstrInfo.inc"

namespace llvm {
class W65816InstrInfo : public W65816GenInstrInfo {
public:
  explicit W65816InstrInfo();
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_W65816_W65816INSTRINFO_H
