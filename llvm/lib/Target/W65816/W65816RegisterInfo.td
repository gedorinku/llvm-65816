let Namespace = "W65816" in {
class W65816Reg<string n, list<string> alt = []> : Register<n> {
  let AltNames = alt;
}

class W65816GPRReg<string n, list<string> alt = []> : W65816Reg<n, alt>;

def A : W65816GPRReg<"a">, DwarfRegNum<[0]>;
def X : W65816GPRReg<"x">, DwarfRegNum<[1]>;
def Y : W65816GPRReg<"y">, DwarfRegNum<[2]>;
def D : W65816GPRReg<"d">, DwarfRegNum<[3]>;
def SP : W65816GPRReg<"sp">, DwarfRegNum<[4]>;

def GPR : RegisterClass<"W65816", [i16], 0, (add
  A,
  X, Y,
  D,
  SP
)> {
  let RegInfos = RegInfoByHwMode<
    [DefaultMode],
    [RegInfo<16, 16, 0>]
}

def GPRA : RegisterClass<"W65816", [i16], 0, (add A)>;

}
