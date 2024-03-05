#ifndef LLVM_CLANG_LIB_BASIC_W65816_H
#define LLVM_CLANG_LIB_BASIC_W65816_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"

namespace clang {
namespace targets {
class W65816TargetInfo : public TargetInfo {
public:
  W65816TargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    TLSSupported = false;
    PointerWidth = 32;
    PointerAlign = 1;
    IntWidth = 16;
    IntAlign = 1;
    LongWidth = 32;
    LongAlign = 1;
    LongLongWidth = 32;
    LongLongAlign = 1;
    SuitableAlign = 1;
    DefaultAlignForAttributeAligned = 0;
    HalfWidth = 8;
    HalfAlign = 8;
    FloatWidth = 32;
    FloatAlign = 1;
    DoubleWidth = 32;
    DoubleAlign = 1;
    DoubleFormat = &llvm::APFloat::IEEEsingle();
    LongDoubleWidth = 32;
    LongDoubleAlign = 1;
    LongDoubleFormat = &llvm::APFloat::IEEEsingle();
    SizeType = UnsignedInt;
    PtrDiffType = SignedInt;
    IntPtrType = SignedInt;
    Char16Type = UnsignedInt;
    WIntType = SignedInt;
    Char32Type = UnsignedLong;
    SigAtomicType = SignedChar;

    resetDataLayout("e-m:e-p:32:32-n16-S8");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool isValidCPUName(StringRef Name) const override;
  bool setCPU(const std::string &Name) override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override { return ArrayRef<Builtin::Info>(); }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  std::string_view getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    return false;
  }
};
} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_W65816_H
