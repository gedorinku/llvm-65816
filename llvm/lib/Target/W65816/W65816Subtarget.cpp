#include "W65816Subtarget.h"

#include "W65816.h"

#include "W65816TargetMachine.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "W65816-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "W65816GenSubtargetInfo.inc"

W65816Subtarget::W65816Subtarget(const Triple &TT, StringRef &CPU,
                                 StringRef &TuneCPU, StringRef &FS,
                                 const W65816TargetMachine &_TM)
    : W65816GenSubtargetInfo(TT, CPU, TuneCPU, FS), TM(_TM), TargetTriple(TT),
      InstrInfo(),
      FrameLowering(initializeSubtargetDependencies(CPU, TuneCPU, FS, TM)),
      RegInfo(*this, 0) {}

W65816Subtarget &W65816Subtarget::initializeSubtargetDependencies(
    StringRef CPU, StringRef TuneCPU, StringRef FS, const TargetMachine &TM) {
  return *this;
}
