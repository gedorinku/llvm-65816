#ifndef LLVM_LIB_TARGET_W65816_MCTARGETDESC_W65816MCASMINFO_H
#define LLVM_LIB_TARGET_W65816_MCTARGETDESC_W65816MCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class W65816MCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit W65816MCAsmInfo(const Triple &TheTriple);
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_W65816_MCTARGETDESC_W65816MCASMINFO_H
