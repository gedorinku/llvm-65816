#include "TargetInfo/W65816TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheW65816Target() {
  static Target TheW65816Target;
  return TheW65816Target;
}


extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeW65816TargetInfo() {
  RegisterTarget<Triple::w65816> X(getTheW65816Target(), "w65816", "W65816",
                                   "W65816");
}
