#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_W65816_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_W65816_H

#include "llvm/Option/Option.h"

namespace clang {
namespace driver {
namespace tools {
namespace w65816 {

std::string getW65816TargetCPU(const llvm::opt::ArgList &Args);

}
} // namespace tools
} // namespace driver
} // namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_W65816_H
