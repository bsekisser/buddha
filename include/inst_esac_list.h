#define INST_ESAC_LIST_MASKED \
	INST(MASKED(0x01, 0x1f), 2, 2, ajmp(addr11), "AJMP") \
	INST(MASKED(0x11, 0x1f), 2, 2, acall(addr11), "ACALL")

#define INST_ESAC_LIST_MISC \
	INST(ESAC(0x00), 0, 0, nop(), "NOP") \
	INST(ESAC(0x02), 3, 2, ljmp(addr16), "LJMP") \
	INST(ESAC(0x03), 0, 0, rr(acc), "RR") \
	INST(ESAC(0x04), 0, 0, inc(acc), "INC") \
	INST(ESAC(0x10), 3, 2, jbc(bit, rel), "JBC") \
	INST(ESAC(0x12), 3, 2, lcall(addr16), "LCALL") \
	INST(ESAC(0x13), 0, 0, rrc(acc), "RRC") \
	INST(ESAC(0x14), 0, 0, dec(acc), "DEC") \
	INST(ESAC(0x20), 3, 2, jb(bit, rel), "JB") \
	INST(ESAC(0x22), 0, 2, ret(), "RET") \
	INST(ESAC(0x23), 0, 0, rl(acc), "RL") \
	INST(ESAC(0x24), 2, 0, add(acc, imm8), "ADD") \
	INST(ESAC(0x30), 3, 2, jnb(bit, rel), "JNB") \
	INST(ESAC(0x32), 0, 2, reti(), "RETI") \
	INST(ESAC(0x33), 0, 0, rlc(acc), "RLC") \
	INST(ESAC(0x34), 2, 0, addc(acc, imm8), "ADDC") \
	INST(ESAC(0x40), 2, 2, jc(rel), "JC") \
	INST(ESAC(0x42), 2, 0, orl(dir, acc), "ORL") \
	INST(ESAC(0x43), 3, 2, orl(dir, imm8), "ORL") \
	INST(ESAC(0x44), 2, 0, orl(acc, imm8), "ORL") \
	INST(ESAC(0x50), 2, 2, jnc(rel), "JNC") \
	INST(ESAC(0x52), 2, 0, anl(dir, acc), "ANL") \
	INST(ESAC(0x53), 3, 2, anl(dir, imm8), "ANL") \
	INST(ESAC(0x54), 2, 0, anl(acc, imm8), "ANL") \
	INST(ESAC(0x60), 2, 2, jz(rel), "JZ") \
	INST(ESAC(0x62), 2, 0, xrl(dir, acc), "XRL") \
	INST(ESAC(0x63), 3, 2, xrl(dir, imm8), "XRL") \
	INST(ESAC(0x64), 2, 0, xrl(acc, imm8), "XRL") \
	INST(ESAC(0x70), 2, 2, jnz(rel), "JNZ") \
	INST(ESAC(0x72), 2, 2, orl(bPSW_CY, bit), "ORL") \
	INST(ESAC(0x73), 0, 2, ljmp(atA_DPTRc), "JMP") \
	INST(ESAC(0x74), 2, 0, mov(acc, imm8), "MOV") \
	INST(ESAC(0x80), 2, 2, sjmp(rel), "SJMP") \
	INST(ESAC(0x82), 2, 2, anl(bPSW_CY, bit), "ANL") \
	INST(ESAC(0x83), 2, 0, movc(acc, atA_PC), "MOVC") \
	INST(ESAC(0x84), 0, 4, div(acc, bbb), "DIV") \
	INST(ESAC(0x90), 3, 2, mov(rDPTR, imm16be), "MOV") \
	INST(ESAC(0x92), 2, 2, mov(bit, bPSW_CY), "MOV") \
	INST(ESAC(0x93), 0, 2, movc(acc, atA_DPTRc), "MOVC") \
	INST(ESAC(0x94), 2, 0, subb(acc, imm8), "SUBB") \
	INST(ESAC(0xa0), 2, 2, orl(bPSW_CY, cbit), "ORL") \
	INST(ESAC(0xa2), 2, 2, mov(bPSW_CY, bit), "MOV") \
	INST(ESAC(0xa3), 0, 2, inc(rDPTR), "INC") \
	INST(ESAC(0xa4), 0, 4, mul(acc, bbb), "MUL") \
	INST(ESAC(0xb0), 2, 2, anl(bPSW_CY, cbit), "ANL") \
	INST(ESAC(0xb2), 2, 0, cpl(bit), "CPL") \
	INST(ESAC(0xb3), 0, 0, cpl(bPSW_CY), "CPL") \
	INST(ESAC(0xb4), 3, 2, cjne(acc, imm8, rel), "CJNE") \
	INST(ESAC(0xc0), 2, 2, ppush(dir), "PUSH") \
	INST(ESAC(0xc2), 2, 0, clr(bit), "CLR") \
	INST(ESAC(0xc3), 0, 0, clr(bPSW_CY), "CLR") \
	INST(ESAC(0xc4), 0, 0, swap(acc), "SWAP") \
	INST(ESAC(0xd0), 2, 2, ppop(dir), "POP") \
	INST(ESAC(0xd2), 2, 0, setb(bit), "SETB") \
	INST(ESAC(0xd3), 0, 0, setb(bPSW_CY), "SETB") \
	INST(ESAC(0xd4), 0, 0, da(acc), "DA") \
	INST(ESAC(0xe0), 0, 2, movx(acc, atDPTRx), "MOVX") \
	INST(ESAC_RANGE(0xe2, 0xe3), 0, 2, movx(acc, atRi), "MOVX") \
	INST(ESAC(0xe4), 0, 0, clr(acc), "CLR") \
	INST(ESAC(0xf0), 0, 2, movx(atDPTRx, acc), "MOVX") \
	INST(ESAC_RANGE(0xf2, 0xf3), 0, 2, movx(atRi, acc), "MOVX")

#define INST_ESAC_LIST_a5 \
	INST(ESAC_RANGE(0xa50010, 0xa5001f), 3, 2, add16(WRx_iWR, WR_iWRy), "ADD16") \
	INST(ESAC_RANGE(0xa50040, 0xa5004f), 3, 2, addc16(WRx_iWR, WR_iWRy), "ADDC16") \
	INST(ESAC_RANGE(0xa50060, 0xa5006f), 3, 0, add16(WRx_WR, WR_WRy), "ADD16s") /* ???? */\
	INST(ESAC_RANGE(0xa50090, 0xa5009f), 3, 0, addc16(WRx_WR, WR_WRy), "ADDC16s") /* ???? */\
	INST(ESAC_RANGE(0xa500c0, 0xa500cf), 3, 0, sub16(WRx_WR, WR_WRy), "SUB16") \
	INST(ESAC_RANGE(0xa500d0, 0xa500df), 3, 2, sub16(WRx_iWR, WR_iWRy), "SUB16") \
	INST(ESAC_RANGE(0xa510, 0xa517), 2, 0, inc16(WRx), "INC16") \
	INST(ESAC_RANGE(0xa518, 0xa51f), 2, 0, dec16(WRx), "DEC16") \
	INST(ESAC_RANGE(0xa560, 0xa56f), 2, 0, cmp16(WRx_WR, WR_WRy), "CMP16") \
	INST(ESAC_RANGE(0xa580, 0xa58f), 2, 0, mov16(WRx_WR, WR_WRy), "MOV16") \
	INST(ESAC_RANGE(0xa590, 0xa59f), 2, 2, mov16(iWRx_WR, iWR_WRy), "MOV16") \
	INST(ESAC_RANGE(0xa5a0, 0xa5af), 2, 2, mov16(WRx_iWR, WR_iWRy), "MOV16")

#define INST_ESAC_LIST_MASKED_a5

#define INST_ESAC_LIST_x5 \
	INST(ESAC(0x05), 2, 0, inc(dir), "INC") \
	INST(ESAC(0x15), 2, 0, dec(dir), "DEC") \
	INST(ESAC(0x25), 2, 0, add(acc, dir), "ADD") \
	INST(ESAC(0x35), 2, 0, addc(acc, dir), "ADDC") \
	INST(ESAC(0x45), 2, 0, orl(acc, dir), "ORL") \
	INST(ESAC(0x55), 2, 0, anl(acc, dir), "ANL") \
	INST(ESAC(0x65), 2, 0, xrl(acc, dir), "XRL") \
	INST(ESAC(0x75), 3, 2, mov(dir, imm8), "MOV") \
	INST(ESAC(0x85), 3, 2, mov(dir, dir), "MOV") \
	INST(ESAC(0x95), 2, 0, subb(acc, dir), "SUBB") \
	INST(ESAC(0xb5), 3, 2, cjne(acc, dir, rel), "CJNE") \
	INST(ESAC(0xc5), 2, 0, xch(acc, dir), "XCH") \
	INST(ESAC(0xd5), 3, 2, djnz(dir, rel), "DJNZ") \
	INST(ESAC(0xe5), 2, 0, mov(acc, dir), "MOV") \
	INST(ESAC(0xf5), 2, 0, mov(dir, acc), "MOV")

#define INST_ESAC_LIST_x67 \
	INST(ESAC_RANGE(0x06, 0x07), 0, 0, inc(atRi), "INC") \
	INST(ESAC_RANGE(0x16, 0x17), 0, 0, dec(atRi), "DEC") \
	INST(ESAC_RANGE(0x26, 0x27), 0, 0, add(acc, atRi), "ADD") \
	INST(ESAC_RANGE(0x36, 0x37), 0, 0, addc(acc, atRi), "ADDC") \
	INST(ESAC_RANGE(0x46, 0x47), 0, 0, orl(acc, atRi), "ORL") \
	INST(ESAC_RANGE(0x56, 0x57), 0, 0, anl(acc, atRi), "ANL") \
	INST(ESAC_RANGE(0x66, 0x67), 0, 0, xrl(acc, atRi), "XRL") \
	INST(ESAC(0x76), 2, 0, mov(atRi, dir), "MOV") \
	INST(ESAC(0x77), 2, 0, mov(atRi, imm8), "MOV") \
	INST(ESAC_RANGE(0x86, 0x87), 2, 2, mov(dir, atRi), "MOV") \
	INST(ESAC_RANGE(0x96, 0x97), 0, 0, subb(acc, atRi), "SUBB") \
	INST(ESAC_RANGE(0xa6, 0xa7), 2, 2, mov(atRi, dir), "MOV") \
	INST(ESAC_RANGE(0xb6, 0xb7), 3, 2, cjne(atRi, imm8, rel), "CJNE") \
	INST(ESAC_RANGE(0xc6, 0xc7), 0, 0, xch(acc, atRi), "XCH") \
	INST(ESAC_RANGE(0xe6, 0xe7), 0, 0, mov(acc, atRi), "MOV") \
	INST(ESAC_RANGE(0xf6, 0xf7), 0, 0, mov(atRi, acc), "MOV")

//	INST(ESAC_RANGE(0xd6, 0xd7), 0, 0, xchd(acc, atRi), "XCHD")

#define INST_ESAC_LIST_x8F \
	INST(ESAC_RANGE(0x08, 0x0f), 0, 0, inc(Rn), "INC") \
	INST(ESAC_RANGE(0x18, 0x1f), 0, 0, dec(Rn), "DEC") \
	INST(ESAC_RANGE(0x28, 0x2f), 0, 0, add(acc, Rn), "ADD") \
	INST(ESAC_RANGE(0x38, 0x3f), 0, 0, addc(acc, Rn), "ADDC") \
	INST(ESAC_RANGE(0x48, 0x4f), 0, 0, orl(acc, Rn), "ORL") \
	INST(ESAC_RANGE(0x58, 0x5f), 0, 0, anl(acc, Rn), "ANL") \
	INST(ESAC_RANGE(0x68, 0x6f), 0, 0, xrl(acc, Rn), "XRL") \
	INST(ESAC_RANGE(0x78, 0x7f), 2, 0, mov(Rn, imm8), "MOV") \
	INST(ESAC_RANGE(0x88, 0x8f), 2, 2, mov(dir, Rn), "MOV") \
	INST(ESAC_RANGE(0x98, 0x9f), 0, 0, subb(acc, Rn), "SUBB") \
	INST(ESAC_RANGE(0xa8, 0xaf), 2, 2, mov(Rn, dir), "MOV") \
	INST(ESAC_RANGE(0xb8, 0xbf), 3, 2, cjne(Rn, imm8, rel), "CJNE") \
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
