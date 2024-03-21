#include "W65816ISelLowering.h"
#include "W65816TargetMachine.h"

using namespace llvm;

W65816TargetLowering::W65816TargetLowering(const W65816TargetMachine &TM,
                                           const W65816Subtarget &STI)
    : TargetLowering(TM) {}

SDValue W65816TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  // TODO
  return Chain;
}

SDValue
W65816TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                  bool IsVarArg,
                                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                                  const SmallVectorImpl<SDValue> &OutVals,
                                  const SDLoc &dl, SelectionDAG &DAG) const {
  // TODO
  return Chain;
}
