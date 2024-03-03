#ifndef LLVM_LIB_TARGET_W65816_W65816TARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_W65816_W65816TARGETOBJECTFILE_H

#include "W65816TargetMachine.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

class W65816TargetObjectFile : public TargetLoweringObjectFileELF {
  const W65816TargetMachine *TM;

public:
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_W65816_W65816TARGETOBJECTFILE_H
