include "W65816InstrInfoFormats.td"

class ArithLogicI16<string instr_asm, RegisterClass RC, Operand Od> :
  W65816Inst<(outs RC:$rd), (ins Od:$imm16), !strconcat(instr_asm, " $imm16"),
  [], IIAlu> {
  let isReMaterializable = 1;
}

def simm16: Operand<i16>, ImmLeaf<i16, [{return isInt<16>(Imm);}]>;

def LDA_I : ArithLogicI16<"lda.w", GPRA, simm16>;
