include "llvm/Target/Target.td"

def : ProcessorModel<"cpu-generic", NoSchedModel, []>;

include "W65816RegisterInfo.td"
// include "W65816Schedule.td"
include "W65816InstrInfo.td"
include "W65816CallingConv.td"

def W65816InstrInfo : InstrInfo;

def W65816 : Target {
  let InstructionSet = W65816InstrInfo;
}
