include "W65816InstrInfoFormats.td"

class ArithLogicI16<string instr_asm, RegisterClass RC, Operand Od> :
  W65816Inst<(outs RC:$rd), (ins Od:$imm16), !strconcat(instr_asm, " $imm16"),
  [], IIAlu> {
  let isReMaterializable = 1;
}

def LDA_I : ArithLogicI16<"lda.w", GPRA, simm16>;
