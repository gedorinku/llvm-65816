#include "W65816.h"
#include "clang/Basic/MacroBuilder.h"
#include "llvm/ADT/StringSwitch.h"

using namespace clang;
using namespace clang::targets;

void W65816TargetInfo::getTargetDefines(const LangOptions &Opts,
                                        MacroBuilder &Builder) const {
  Builder.defineMacro("__w65816__");
}

bool W65816TargetInfo::isValidCPUName(StringRef Name) const {
  return Name == "generic";
}

bool W65816TargetInfo::setCPU(const std::string &Name) {
  return isValidCPUName(Name);
}

ArrayRef<const char *> W65816TargetInfo::getGCCRegNames() const {
  static const char *const GCCRegNames[] = {"a", "x", "y", "d", "sp"};
  return llvm::ArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::GCCRegAlias> W65816TargetInfo::getGCCRegAliases() const {
  static const TargetInfo::GCCRegAlias GCCRegAliases[] = {
      {{}, "a"}, {{}, "x"}, {{}, "y"}, {{}, "sp"}};
  return llvm::ArrayRef(GCCRegAliases);
}
