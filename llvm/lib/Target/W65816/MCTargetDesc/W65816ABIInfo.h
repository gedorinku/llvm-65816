#ifndef LLVM_LIB_TARGET_W65816_MCTARGETDESC_W65816ABIINFO_H
#define LLVM_LIB_TARGET_W65816_MCTARGETDESC_W65816ABIINFO_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/MC/MCRegisterInfo.h"

namespace llvm {
class W65816ABIInfo {
public:
  enum class ABI { Unknown, STACK };

protected:
  ABI ThisABI;

public:
  W65816ABIInfo(ABI ThisABI) : ThisABI(ThisABI) {}


  static W65816ABIInfo Unknown() { return W65816ABIInfo(ABI::Unknown); }
  static W65816ABIInfo STACK() { return W65816ABIInfo(ABI::STACK); }
  static W65816ABIInfo computeTargetABI(StringRef ABIName);
  // @{ MYRISCVXABIInfo_h_MYRISCVXABIInfo ...

  bool IsKnown() const { return ThisABI != ABI::Unknown; }
  bool IsSTACK() const { return ThisABI == ABI::STACK; }
  ABI GetEnumValue() const { return ThisABI; }

  /// The registers to use for byval arguments.
  ArrayRef<MCPhysReg> GetByValArgRegs() const;

  /// The registers to use for the variable argument list.
  ArrayRef<MCPhysReg> GetVarArgRegs() const;

  /// Obtain the size of the area allocated by the callee for arguments.
  /// CallingConv::FastCall affects the value for O32.
  unsigned GetCalleeAllocdArgSizeInBytes(CallingConv::ID CC) const;

  /// Ordering of ABI's
  /// MYRISCVXGenSubtargetInfo.inc will use this to resolve conflicts when given
  /// multiple ABI options.
  bool operator<(const W65816ABIInfo Other) const {
    return ThisABI < Other.GetEnumValue();
  }

  // @} MYRISCVXABIInfo_h_MYRISCVXABIInfo ...
  unsigned GetStackPtr() const;
  unsigned GetFramePtr() const;
  unsigned GetNullPtr() const;

  unsigned GetEhDataReg(unsigned I) const;
  int EhDataRegSize() const;
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_W65816_MCTARGETDESC_W65816ABIINFO_H
