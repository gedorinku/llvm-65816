#include "W65816TargetObjectFile.h"

using namespace llvm;

void W65816TargetObjectFile::Initialize(MCContext &Ctx,
                                        const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
}
