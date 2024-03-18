#ifndef LLVM_LIB_TARGET_W65816_W65816ASMPRINTER_H
#define LLVM_LIB_TARGET_W65816_W65816ASMPRINTER_H

#include "W65816TargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class LLVM_LIBRARY_VISIBILITY W65816AsmPrinter : public AsmPrinter {
public:
  explicit W65816AsmPrinter(TargetMachine &TM,
                            std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_W65816_W65816ASMPRINTER_H
