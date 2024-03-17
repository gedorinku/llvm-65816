#include "W65816MCTargetDesc.h"
#include "MCTargetDesc/W65816InstPrinter.h"
#include "W65816MCAsmInfo.h"

#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "W65816GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "W65816GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "W65816GenRegisterInfo.inc"

static MCInstrInfo *createW65816MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitW65816MCInstrInfo(X); // defined in W65816GenInstrInfo.inc
  return X;
}

static MCRegisterInfo *createW65816MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitW65816MCRegisterInfo(X, 0); // defined in W65816GenRegisterInfo.inc
  return X;
}

static MCSubtargetInfo *
createW65816MCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  std::string CPUName = std::string(CPU);
  if (CPU.empty())
    CPU = TT.isArch64Bit() ? "cpu-rv64" : "cpu-rv32";
  return createW65816MCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPUName, FS);
}

static MCAsmInfo *createW65816MCAsmInfo(const MCRegisterInfo &MRI,
                                        const Triple &TT,
                                        const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new W65816MCAsmInfo(TT);

  unsigned SP = MRI.getDwarfRegNum(W65816::SP, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCInstPrinter *createW65816MCInstPrinter(const Triple &T,
                                                unsigned SyntaxVariant,
                                                const MCAsmInfo &MAI,
                                                const MCInstrInfo &MII,
                                                const MCRegisterInfo &MRI) {
  return new W65816InstPrinter(MAI, MII, MRI);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeW65816TargetMC() {
  Target *T = &getTheW65816Target();

  RegisterMCAsmInfoFn X(*T, createW65816MCAsmInfo);

  // MCInstInfoクラスを登録する
  TargetRegistry::RegisterMCInstrInfo(*T, createW65816MCInstrInfo);

  // MCRegisterInfoクラスを登録する
  TargetRegistry::RegisterMCRegInfo(*T, createW65816MCRegisterInfo);

  TargetRegistry::RegisterMCSubtargetInfo(*T, createW65816MCSubtargetInfo);

  // MCInstrAnalysisクラスを登録する
  //    TargetRegistry::RegisterMCInstrAnalysis(*T,
  //    createW65816MCInstrAnalysis);
  // MCInstPrinterクラスを登録する
  TargetRegistry::RegisterMCInstPrinter(*T, createW65816MCInstPrinter);

  // アセンブリファイル出力用TargetStreamerの登録
  //    TargetRegistry::RegisterAsmTargetStreamer(*T,
  //    createW65816AsmTargetStreamer);

  // オブジェクトファイルのTarget Streamerを登録する
  //    TargetRegistry::RegisterObjectTargetStreamer(*T,
  //    createW65816ObjectTargetStreamer);

  // MC Code Emitterを登録する
  //    TargetRegistry::RegisterMCCodeEmitter(*T, createW65816MCCodeEmitter);

  // MCAsmBackendを登録する
  //    TargetRegistry::RegisterMCAsmBackend(*T, createW65816AsmBackend);
}
