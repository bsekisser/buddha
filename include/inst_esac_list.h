#define INST_ESAC_LIST_MASKED \
	INST(MASKED(0x01, 0x1f), 2, 2, ajmp(addr11), "AJMP") \
	INST(MASKED(0x11, 0x1f), 2, 2, acall(addr11), "ACALL")

#define INST_ESAC_LIST_MISC \
	INST(ESAC(0x00), 0, 0, nop(), "NOP") \
	INST(ESAC(0x02), 3, 2, ljmp(addr16), "LJMP") \
	INST(ESAC(0x12), 3, 2, lcall(addr16), "LCALL") \
	INST(ESAC(0x20), 3, 2, jb(bit, rel), "JB") \
	INST(ESAC(0x22), 0, 2, ret(), "RET") \
	INST(ESAC(0x30), 3, 2, jnb(bit, rel), "JNB") \
	INST(ESAC(0x40), 2, 2, jc(rel), "JC") \
	INST(ESAC(0x42), 2, 0, orl(dir, acc), "ORL") \
	INST(ESAC(0x43), 3, 2, orl(dir, imm8), "ORL") \
	INST(ESAC(0x50), 2, 2, jnc(rel), "JNC") \
	INST(ESAC(0x53), 3, 2, anl(dir, imm8), "ANL") \
	INST(ESAC(0x54), 2, 0, anl(acc, imm8), "ANL") \
	INST(ESAC(0x60), 2, 2, jz(rel), "JZ") \
	INST(ESAC(0x70), 2, 2, jnz(rel), "JNZ") \
	INST(ESAC(0x73), 0, 2, ljmp(atA_DPTR), "JMP") \
	INST(ESAC(0x74), 2, 0, mov(acc, imm8), "MOV") \
	INST(ESAC(0x80), 2, 2, sjmp(rel), "SJMP") \
	INST(ESAC(0x90), 3, 2, mov(rDPTR, imm16be), "MOV") \
	INST(ESAC(0xa3), 2, 0, inc(rDPTR), "INC") \
	INST(ESAC(0xc0), 2, 2, ppush(dir), "PUSH") \
	INST(ESAC(0xc2), 2, 0, clr(bit), "CLR") \
	INST(ESAC(0xc3), 0, 0, clr(bPSW_CY), "CLR") \
	INST(ESAC(0xc4), 0, 0, swap(acc), "SWAP") \
	INST(ESAC(0xd0), 2, 2, ppop(dir), "POP") \
	INST(ESAC(0xd2), 2, 0, setb(bit), "SETB") \
	INST(ESAC(0xe0), 0, 2, mov(acc, atDPTR), "MOVX") \
	INST(ESAC(0xe4), 0, 0, clr(acc), "CLR") \
	INST(ESAC(0xf0), 0, 2, movx(atDPTR, acc), "MOVX")

#define INST_ESAC_LIST_a5 \
		INST(ESAC_RANGE(0xa510, 0xa517), 2, 0, inc16(x2), "INC16") \
		INST(ESAC_RANGE(0xa560, 0xa56f), 2, 0, cmp16(X2y2, x2Y2), "CMP16")

#define INST_ESAC_LIST_a5_MASKED

#define INST_ESAC_LIST_x5 \
	INST(ESAC(0x25), 2, 0, add(acc, dir), "ADD") \
	INST(ESAC(0x35), 2, 0, adc(acc, dir), "ADDC") \
	INST(ESAC(0x75), 3, 2, mov(dir, imm8), "MOV") \
	INST(ESAC(0xc5), 2, 0, xch(acc, dir), "XCH") \
	INST(ESAC(0xf5), 2, 0, mov(dir, acc), "MOV")

#define INST_ESAC_LIST_x67 \
	INST(ESAC_RANGE(0xe6, 0xe7), 0, 0, mov(acc, atRi), "MOV") \

#define INST_ESAC_LIST_x8F \
	INST(ESAC_RANGE(0x08, 0x0f), 0, 0, inc(Rn), "INC") \
	INST(ESAC_RANGE(0x78, 0x7f), 2, 0, mov(Rn, imm8), "MOV") \
	INST(ESAC_RANGE(0x88, 0x8f), 2, 2, mov(dir, Rn), "MOV") \
	INST(ESAC_RANGE(0xa8, 0xaf), 2, 2, mov(Rn, dir), "MOV") \
	INST(ESAC_RANGE(0xc8, 0xcf), 0, 0, xch(acc, Rn), "XCH") \
	INST(ESAC_RANGE(0xd8, 0xdf), 2, 2, djnz(Rn, rel), "DJNZ") \
	INST(ESAC_RANGE(0xe8, 0xef), 0, 0, mov(acc, Rn), "MOV") \
	INST(ESAC_RANGE(0xf8, 0xff), 0, 0, mov(Rn, acc), "MOV")

/* **** */

#define INST_ESAC_LIST \
	INST_ESAC_LIST_MISC \
	INST_ESAC_LIST_x5 \
	INST_ESAC_LIST_x67 \
	INST_ESAC_LIST_x8F
