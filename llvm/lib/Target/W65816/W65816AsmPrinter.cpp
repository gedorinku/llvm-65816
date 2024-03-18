#include "W65816AsmPrinter.h"

#include "W65816.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

extern "C" void LLVMInitializeW65816AsmPrinter() {
  RegisterAsmPrinter<W65816AsmPrinter> X(getTheW65816Target());
}
