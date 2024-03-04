#include "W65816ABIInfo.h"
#include "W65816RegisterInfo.h"

using namespace llvm;

W65816ABIInfo W65816ABIInfo::computeTargetABI(StringRef ABIName) {
  return W65816ABIInfo::ABI::STACK;
}

ArrayRef<MCPhysReg> W65816ABIInfo::GetByValArgRegs() const {
  return ArrayRef<MCPhysReg>();
}

ArrayRef<MCPhysReg> W65816ABIInfo::GetVarArgRegs() const {
  return ArrayRef<MCPhysReg>();
}

unsigned
W65816ABIInfo::GetCalleeAllocdArgSizeInBytes(CallingConv::ID CC) const {
  return 0;
}

unsigned W65816ABIInfo::GetStackPtr() const { return W65816::SP; }

unsigned W65816ABIInfo::GetFramePtr() const { return 0; }

unsigned W65816ABIInfo::GetNullPtr() const { return 0; }

unsigned W65816ABIInfo::GetEhDataReg(unsigned int I) const { return 0; }

int W65816ABIInfo::EhDataRegSize() const { return 0; }
