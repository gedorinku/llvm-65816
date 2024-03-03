class W65816Inst<dag outs, dag ins, string asmstr, list<dag> pattern,
        InstrItinClass itin>: Instruction
{
  let Namespace = "W65816";

  let OutOperandList = outs;
  let InOperandList = ins;

  let AsmString = asmstr;
  let Pattern = pattern;
  let Itinerary = itin;

  let DecoderNamespace = "W65816";
}

// TODO
def IIAlu              : InstrItinClass;
def IILoad             : InstrItinClass;
def IIStore            : InstrItinClass;
def IIBranch           : InstrItinClass;

def IIPseudo           : InstrItinClass;