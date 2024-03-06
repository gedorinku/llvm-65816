#ifndef LLVM_LIB_TARGET_W65816_MCTARGETDESC_W65816MCTARGETDESC_H
#define LLVM_LIB_TARGET_W65816_MCTARGETDESC_W65816MCTARGETDESC_H

namespace llvm {
class Target;

Target &getTheW65816Target();
}

#define GET_REGINFO_ENUM
#include "W65816GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "W65816GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "W65816GenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_W65816_MCTARGETDESC_W65816MCTARGETDESC_H
