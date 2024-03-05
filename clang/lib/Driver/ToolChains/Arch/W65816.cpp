#include "W65816.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace llvm::opt;

std::string w65816::getW65816TargetCPU(const llvm::opt::ArgList &Args) {
  // If we have -mcpu, use that.
  if (Arg *A = Args.getLastArg(options::OPT_mcpu_EQ)) {
    return A->getValue();
  }

  // Otherwise use 'generic'.
  return "generic";
}
