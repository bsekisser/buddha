#define INST_ESAC_LIST_MASKED

#define INST_ESAC_LIST_MISC \
	INST(ESAC(0x00), 0, 0, nop(), "NOP") \
	INST(ESAC(0x12), 3, 2, lcall(addr16), "LCALL") \
	INST(ESAC(0x73), 0, 2, ljmp(atA_DPTR), "JMP") \
	INST(ESAC(0x74), 2, 0, mov(acc, imm8), "MOV") \
	INST(ESAC(0xc0), 2, 2, ppush(dir), "PUSH") \
	INST(ESAC(0xc2), 2, 0, clr(bit), "CLR") \
	INST(ESAC(0xd0), 2, 2, ppop(dir), "POP") \
	INST(ESAC(0xe4), 0, 0, clr(acc), "CLR") \

#define INST_ESAC_LIST_x5 \
	INST(ESAC(0x25), 2, 0, add(acc, dir), "ADD") \

#define INST_ESAC_LIST_x67 \
	INST(ESAC_RANGE(0xe6, 0xe7), 0, 0, mov(acc, atRi), "MOV") \

#define INST_ESAC_LIST_x8F \
	INST(ESAC_RANGE(0x08, 0x0f), 0, 0, inc(Rn), "INC") \
	INST(ESAC_RANGE(0x78, 0x7f), 2, 0, mov(Rn, imm8), "MOV") \
	INST(ESAC_RANGE(0xa8, 0xaf), 2, 2, mov(Rn, dir), "MOV") \
	INST(ESAC_RANGE(0xc8, 0xcf), 0, 0, xch(acc, Rn), "XCH") \
	INST(ESAC_RANGE(0xd8, 0xdf), 2, 2, djnz(Rn, rel), "DJNZ") \
	INST(ESAC_RANGE(0xe8, 0xef), 0, 0, mov(acc, Rn), "MOV") \

/* **** */

#define INST_ESAC_LIST \
	INST_ESAC_LIST_MISC \
	INST_ESAC_LIST_x5 \
	INST_ESAC_LIST_x67 \
	INST_ESAC_LIST_x8F
