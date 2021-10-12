#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/bitfield.h"

const char spaces[61] = "                                                             ";

#include "vm8051_trace.h"

#define _JMP(_ea) (~0xffffUL + (_ea & 0xffffUL))
#define JMP(_ea) PC = _JMP(_ea);

#define _RJMP(_ea) (PC + (int8_t)_ea)
#define RJMP(_ea) PC = _RJMP((int8_t)_ea);

#include "on_err.h"
#include "vm8051.h"
#include "vm8051_ea.h"
#include "vm8051_ir.h"

/* from https://github.com/Spritetm/unbuddha/unbuddha.c */
typedef struct code_idx_ent_t* code_idx_ent_p;
typedef struct __attribute__((packed)) code_idx_ent_t {
	uint16_t idx;
	uint16_t len;
	uint16_t unk1; //0, possibly part of len
	uint16_t load_at;
	uint16_t unk2; //0, possibly part of load_at
	uint16_t offset;
	uint16_t dcrc;
	uint16_t tcrc;
} code_idx_ent_t;

void _add_set_flags(vm_p vm, uint8_t v1, uint8_t v2, uint8_t res)
{
	uint32_t carry = v1 ^ v2 ^ res;

	BSET_AS(PSW, PSW_BIT_CY, (carry >> 7) & 1);
	BSET_AS(PSW, PSW_BIT_AC, (carry >> 3) & 1);
}

static uint16_t bswap16(uint16_t data)
{
	return((data >> 8) | (data << 8));
}

/* **** */

uint8_t ld(vm_p vm, uint32_t pat)
{
	uint8_t segment = (pat >> 16) & 0xff;
	uint8_t page = (pat >> 8) & 0xff;
	
	switch(segment) {
		case 0xff:
			if(page <= 0x1f)
				return(vm->irom[pat & 0x1fff]);
			else
				return(vm->xrom[pat & 0xffff]);
		break;
		case 0x01:
			return(vm->xram[pat & 0xffff]);
		break;
		case 0x00:
			if(pat < 0x80)
				return(vm->iram[pat & 0xff]);
			else
			{
				switch(pat & 0xff)
				{
					case _SFR_SP:		/* 0x81 */
						return(SP & 0xff);
					break;
					case _SFR_DPL:		/* 0x82 */
						return(DPTR & 0xff);
					break;
					case _SFR_DPH:		/* 0x83 */
						return(DPTR >> 8);
					break;
					case _SFR_SPH:		/* 0xbe */
						return(SP >> 8);
					break;
/* **** */
					case 0x86:
					case _SFR_IE:		/* 0xa8 -- interrupt control */
					case 0xbc:			/* ? C1CAP2H ? */
					case 0xbd:			/* ? C1CAP3H ? */
					case _SFR_PSW:		/* 0xd0 */
					case _SFR_ACC:		/* 0xe0 */
					case _SFR_B:		/* 0xf0 */
						return(_SFR_(pat));
					break;
					default:
						TRACE("unhandled sfr (0x%02X)", pat & 0xff);
//						exit(-1);
					break;
				}
			}
		break;
	}

	return((pat >> 8) & 0xff);
}

void st(vm_p vm, uint32_t pat, uint8_t data)
{
	uint8_t segment = (pat >> 16) & 0xff;
	uint8_t page = (pat >> 8) & 0xff;

	switch(segment) {
		case 0x01:
			vm->xram[pat & 0xffff] = data;
		break;
		case 0x00:
			if(pat < 80)
				vm->iram[pat & 0xff] = data;
			else
			{
				switch(pat & 0xff)
				{
					case _SFR_SP:		/* 0x81 */
						SP = (SP & ~0xff) | data;
					break;
					case _SFR_DPL:		/* 0x82 */
						DPTR = (DPTR & ~0xff) | data;
					break;
					case _SFR_DPH:		/* 0x83 */
						DPTR = (DPTR & ~0xff00) | (data << 8);
					break;
					case _SFR_SPH:		/* 0xb3 */
						SP = (SP & ~0xff00) | (data << 8);
					break;
/* **** */
					case 0x86:
					case _SFR_PCON:		/* 0x87 */
					case _SFR_IE:		/* 0xa8 -- interrupt control */
					case 0xbc:			/* ? C1CAP2H ? */
					case 0xbd:			/* ? C1CAP3H ? */
					case _SFR_PSW:		/* 0xd0 */
					case _SFR_ACC:		/* 0xe0 */
					case _SFR_B:		/* 0xf0 */
						_SFR_(pat) = data;
					break;
					default:
						TRACE("unhandled sfr (0x%02X)", pat & 0xff);
//						exit(-1);
					break;
				}
			}
		break;
	}
}

/* **** */

uint8_t bit_read(vm_p vm, uint8_t pat)
{
	const uint8_t ea = pat & ~0x7f;
	const uint8_t pos = pat & 7;
	
	return((ld(vm, ea) >> pos) & 1);
}

void bit_write(vm_p vm, uint8_t pat, uint8_t set)
{
	const uint8_t ea = pat & ~0x7f;
	const uint8_t pos = pat & 7;
	const uint8_t bit = 1 << bit;
	const uint8_t mask = ~bit;
	
	uint8_t data = ld(vm, ea) & mask;
	data |= bit;
	
	st(vm, ea, data);
}

static uint8_t direct_read(vm_p vm, uint8_t pat)
{
	return(ld(vm, pat));
}

static void direct_write(vm_p vm, uint8_t pat, uint8_t data)
{
	st(vm, pat, data);
}

static uint8_t indirect_read(vm_p vm, uint8_t pat)
{
	return(ld(vm, _Rn_(vm, pat & 1)));
}

static void indirect_write(vm_p vm, uint8_t pat, uint8_t data)
{
	st(vm, _Rn_(vm, pat & 1), data);
}

/* **** */

static uint32_t ld_da(vm_p vm, uint32_t* ppat, int count)
{
	uint32_t data = 0;
	
	for(int i = count; i; i--)
		data = data << 8 | ld(vm, (*ppat)--);

	return(data);
}

uint32_t ld_ia(vm_p vm, uint32_t *ppat, int count)
{
	uint32_t data = 0;
	
	for(int i = 0; i < count; i++)
		data |= (ld(vm, (*ppat)++) << (i << 3));

	return(data);
}
	

uint32_t ld_ia_be(vm_p vm, uint32_t* ppat, int count)
{
	uint32_t data = 0;
	
	for(int i = count; i; i--)
		data = data << 8 | ld(vm, (*ppat)++);

	return(data);
}

uint32_t pop(vm_p vm, int count)
{
	uint32_t data = 0;

	for(int i = count; i; i--)
	{
		data = (data << 8) | vm->iram[SP] & 0xff;
		SP = (SP - 1) & 0xff;
	}

	return(data);
}

void push(vm_p vm, uint32_t data, int count)
{
	for(int i = count; i; i--)
	{
		SP = (SP + 1) & 0xff;
		vm->iram[SP] = data & 0xff;
		data >>= 8;
	}
}

static void vm_reset(vm_p vm)
{
	/* X = undefined */
	
	ACC = 0;
//	B = 0;
	PSW = 0;
	SP = 7;
	DPXL = 1;
	DPTR = 0;
	SFR(P0) = 0xff;
	SFR(P1) = 0xff;
	SFR(P2) = 0xff;
	SFR(P3) = 0xff;
//	IP = is8051 ? XXX00000 : (is8052 ? XX000000 : 0)
//	IE = is8051 ? 0XX00000 : (is8052 ? 0X000000 : 0)
//	TMOD = 0;
//	TCON = 0;
//	if8052(T2CON = 0);
//	TH0 = 0;
//	TL0 = 0;
//	TH1 = 0;
//	TL1 = 0;
//	if8052(TH2 = 0);
//	if8052(TL2 = 0);
//	if8052(RCAP2H = 0);
//	if8052(RCAP2L = 0);
//	SCON = 0;
//	PCON = isHMOS ? 0XXXXXXX : (isCHMOS ? 0XXX0000 : 0);
	PC = 0xffff0000;
}

enum {
	_adc_k,
	_add_k,
	_and_k,
	_clr_k,
	_dec_k,
	_inc_k,
	_or_k,
	_setb_k,
	_sub_k,
	_xor_k,
	_flags = 0x80,
};

static uint8_t _alu_box(vm_p vm, int esac)
{
	uint8_t res = ARG(0)->v;
	
	switch(esac & 0xf) {
		case	_add_k:
			res += ARG(1)->v;
			CODE_TRACE_ALU(+);
			_add_set_flags(vm, ARG(0)->v, ARG(1)->v, res);
		break;
		case	_adc_k:
			res += ARG(1)->v + !!PSW_CY;
			CODE_TRACE_ALU(+);
			_add_set_flags(vm, ARG(0)->v, ARG(1)->v, res);
		break;
		case	_and_k:
			res &= ARG(1)->v;
			CODE_TRACE_ALU(&);
		break;
		case	_clr_k:
			res &= ARG(0)->bit.mask;
			CODE_TRACE_COMMENT("0x%02X & ~0x%02X --> 0x%02X", ARG(0)->v, ARG(0)->bit.bit, res);
		break;
		case	_dec_k:
			res -= ARG(1)->v;
			CODE_TRACE_ALU(--);
		break;
		case	_inc_k:
			res += ARG(1)->v;
			CODE_TRACE_ALU(++);
		break;
		case	_or_k:
			res |= ARG(1)->v;
			CODE_TRACE_ALU(|);
		break;
		case	_setb_k:
			res |= ARG(0)->bit.bit;
			CODE_TRACE_COMMENT("0x%02X | 0x%02X --> 0x%02X", ARG(0)->v, ARG(0)->bit.bit, res);
		break;
		case	_sub_k:
			res -= ARG(1)->v;
			CODE_TRACE_ALU(-);
			_add_set_flags(vm, ARG(0)->v, -ARG(1)->v, res);
		break;
		case	_xor_k:
			res ^= ARG(1)->v;
			CODE_TRACE_ALU(^);
		break;
	}

	return(res);
}

static void vm_step(vm_p vm)
{
	uint16_t IRxA5 = 0;
	
	IP = PC;

_fetch_ir:
	IR = IRxA5 | ld_ia(vm, &PC, 1);

	CODE_TRACE_START();

	CYCLE += 1;

	uint8_t res;

	switch(IR) /* common decode / setup */
	{
		case 0x40: /* 2B, 2C */
		case 0x50: /* 2B, 2C */
		case 0x60: /* 2B, 2C */
		case 0x70: /* 2B, 2C */
		case 0x80: /* 2B, 2C */
			SETUP_IR_REL(ARG(0));

		case 0x04:
		case 0x14:
			SETUP_IR_ACC_V(ARG(0), ARG(1), 1);
		break;
		case 0x54: /* 2B */
		case 0x64: /* 2B */
		case 0x94: /* 2B */
			SETUP_IR_ACC_IMM8(ARG(0), ARG(1));
		break;
		case 0xa5:
			IRxA5 = 0xa500;
			goto _fetch_ir;
		break;
		case 0xc5:
			SETUP_IR_ACC_DIR(ARG(0), ARG(1), 1);
		break;
		case 0x26 ... 0x27:
		case 0x36 ... 0x37:
		case 0x56 ... 0x57:
			SETUP_IR_ACC_INDIRECT(ARG(0), ARG(1), 1);
		break;
		case 0x08 ... 0x0f:
		case 0x18 ... 0x1f:
			SETUP_IR_Rn_V(ARG(0), ARG(1), 1);
		break;
		case 0x28 ... 0x2f:
		case 0x38 ... 0x3f:
		case 0x48 ... 0x4f:
		case 0x68 ... 0x6f:
		case 0xc8 ... 0xcf:
			SETUP_IR_ACC_Rn(ARG(0), ARG(1));
		break;
		case 0xa569:
//			SETUP_IR_WRj_atDRk_Disp16(ARG(0), ARG(1));
		break;
	}

	switch(IR)
	{
		case 0x00: /* nop */
			CODE_TRACE_OP(0, "NOP", "", "");
		break;
		case 0x02: /* 3B, 2C */
		case 0x12: {
			CYCLE++;
			SETUP_IR_ADDR16(ARG(0));
			const int is_call = IR & 0x10;
			const char* lcjs = is_call ? "LCALL" : "LJMP";
			CODE_TRACE_OP(2, lcjs, "0x%08X", _JMP(ARG(0)->ea));
			if(is_call) {
				push(vm, PC, 2);
				CODE_TRACE_COMMENT("0x%04X", SP);
			}
			JMP(ARG(0)->ea);
		}break;
		case 0x04:
			CODE_TRACE_OP(0, "INC", "A");
			ACC = _alu_box(vm, _inc_k);
		break;
		case 0x08 ... 0x0f:
			CODE_TRACE_OP(0, "INC", "R%01u");
			_IR_Rn_(vm) = _alu_box(vm, _inc_k);
		break;
		case 0x10:
			SETUP_IR_BIT_REL(ARG(0), 1, ARG(1));
			CODE_TRACE_OP(2, "JBC", "$%02X.%01X, 0x%08X",
				ARG(0)->bit.ea, ARG(0)->bit.pos, _RJMP(ARG(1)->ea));
			if(ARG(0)->v & ARG(0)->bit.bit)
			{
				direct_write(vm, ARG(0)->bit.ea, ARG(0)->v & ARG(0)->bit.mask);
				RJMP(ARG(1)->ea);
			}
		break;
		case 0x14:
			CODE_TRACE_OP(0, "DEC", "A");
			ACC = _alu_box(vm, _dec_k);
		break;
		case 0x22: /* 2C */
			CYCLE++;
			CODE_TRACE_OP(0, "RET", "");
			JMP(pop(vm, 2));
			CODE_TRACE_COMMENT("0x%04X", SP);
		break;
		case 0x26 ... 0x27: /* 2B */
			CODE_TRACE_OP(0, "ADD", "A, @R%01u", IR_Ri);
			ACC = _alu_box(vm, _add_k);
		break;
		case 0x28 ... 0x2f:
			CODE_TRACE_OP(0, "ADD", "A, R%01u", IR_Rn);
			ACC = _alu_box(vm, _add_k);
		break;
		case 0x33:
			CODE_TRACE_OP(0, "RLC", "A");
			res = ACC << 1 | !!PSW_CY;
			BSET_AS(PSW, PSW_BIT_CY, (ACC >> 7) & 1);
			CODE_TRACE_COMMENT("0x%02X << 1 --> 0x%02X, CY = %01u", ACC, res, PSW_CY);
			ACC = res;
		break;
		case 0x36 ... 0x37: /* 2B */
			CODE_TRACE_OP(0, "ADDC", "A, @R%01u", IR_Ri);
			ACC = _alu_box(vm, _adc_k);
		break;
		case 0x38 ... 0x3f:
			CODE_TRACE_OP(0, "ADDC", "A, R%01u", IR_Rn);
			ACC = _alu_box(vm, _adc_k);
		break;
		case 0x40: /* 2B, 2C */
			CYCLE++;
			CODE_TRACE_OP(1, "JC", "0x%08X", _RJMP(ARG(0)->ea));
			res = !!PSW_CY;
			CODE_TRACE_COMMENT("CY == %01u, will%sjump", PSW_CY, res ? " " : " not ");
			if(res)
				RJMP(ARG(0)->ea);
		break;
		case 0x42: /* 2B */
			SETUP_IR_DIR_ACC(ARG(0), 1, ARG(1));
			CODE_TRACE_OP(1, "ORL", "$%02X, A", ARG(0)->ea);
			res = _alu_box(vm, _or_k);
			direct_write(vm, ARG(0)->ea, res);
		break;
		case 0x43: /* 3B, 2C */
			SETUP_IR_DIR_IMM8(ARG(0), 1, ARG(1));
			CODE_TRACE_OP(2, "ORL", "$%02X, #%02X", ARG(0)->ea, ARG(1)->v);
			res = _alu_box(vm, _or_k);
			direct_write(vm, ARG(0)->ea, res);
		break;
		case 0x48 ... 0x4f:
			CODE_TRACE_OP(0, "ORL", "A, R%01u", IR_Rn);
			ACC = _alu_box(vm, _or_k);
		break;
		case 0x50: /* 2B, 2C */
			CYCLE++;
			CODE_TRACE_OP(1, "JNC", "0x%08X", _RJMP(ARG(0)->ea));
			res = !(!!PSW_CY);
			CODE_TRACE_COMMENT("CY == %01u, will%sjump", PSW_CY, res ? " " : " not ");
			if(res)
				RJMP(ARG(0)->ea);
		break;
		case 0x53: /* 3B, 2C */
			CYCLE++;
			SETUP_IR_DIR_IMM8(ARG(0), 1, ARG(1));
			CODE_TRACE_OP(2, "ANL", "$%02X, #%02X", ARG(0)->ea, ARG(1)->v);
			res = _alu_box(vm, _and_k);
			direct_write(vm, ARG(0)->ea, res);
		break;
		case 0x54: /* 2B */
			CODE_TRACE_OP(1, "ANL", "A, #%02X", ARG(1)->v);
			ACC = _alu_box(vm, _and_k);
		break;
		case 0x56 ... 0x57:
			CODE_TRACE_OP(0, "ANL", "A, @R%01X", ARG(1)->ea);
			ACC = _alu_box(vm, _and_k);
		break;
		case 0x60: /* 2B, 2C */
			CYCLE++;
			CODE_TRACE_OP(1, "JZ", "0x%08X", _RJMP(ARG(0)->ea));
			res = (0 == ACC);
			CODE_TRACE_COMMENT("will%sjump", res ? " " : " not ");
			if(res)
				RJMP(ARG(0)->ea);
		break;
		case 0x64: /* 2B */
			CODE_TRACE_OP(1, "XRL", "A, #%02X", ARG(1)->v);
			ACC = _alu_box(vm, _xor_k);
		break;
		case 0x68 ... 0x6f:
			CODE_TRACE_OP(0, "XRL", "A, R%01u", IR_Rn);
			ACC = _alu_box(vm, _xor_k);
		break;
		case 0x70: /* 2B, 2C */
			CYCLE++;
			CODE_TRACE_OP(1, "JNZ", "0x%08X", _RJMP(ARG(0)->ea));
			res = (0 != ACC);
			CODE_TRACE_COMMENT("will%sjump", res ? " " : " not ");
			if(res)
				RJMP(ARG(0)->ea);
		break;
		case 0x74: /* 2B */
			ACC = ld_ia(vm, &PC, 1);
			CODE_TRACE_OP(1, "MOV", "A, #%02X", ACC);
		break;
		case 0x75: /* 3B, 2C */
			CYCLE++;
			SETUP_IR_DIR_IMM8(ARG(0), 0, ARG(1));
			CODE_TRACE_OP(2, "MOV", "$%02X, #%02X", ARG(0)->ea, ARG(1)->v);
			direct_write(vm, ARG(0)->ea, ARG(1)->v);
		break;
		case 0x78 ... 0x7f: /* 2B */
			SETUP_IR_IMM8(ARG(1));
			CODE_TRACE_OP(1, "MOV", "R%01u, #%02X", IR_Rn, ARG(1)->v);
			_IR_Rn_(vm) = ARG(1)->v;
		break;
		case 0x80: /* 2B, 2C */
			CYCLE++;
			CODE_TRACE_OP(1, "SJMP", "0x%08X", _RJMP(ARG(0)->ea));
			RJMP(ARG(0)->ea);
		break;
		case 0x88 ... 0x8f: /* 2B, 2C */
			CYCLE++;
			SETUP_IR_DIR_Rn(ARG(0), 0, ARG(1));
			CODE_TRACE_OP(1, "MOV", "$%02X, R%01u", ARG(0)->ea, IR_Rn);
			CODE_TRACE_COMMENT("0x%02X", ARG(1)->v);
			direct_write(vm, ARG(0)->ea, ARG(1)->v);
		break;
		case 0x90: /* 3B, 2C */
			CYCLE++;
			DPTR = ld_ia_be(vm, &PC, 2);
			CODE_TRACE_OP(2, "MOV", "DPTR, #%04X", DPTR);
		break;
		case 0x93: { /* 2C */
			CYCLE++;
			uint16_t ea = ACC + DPTR;
			res = ld(vm, 0x00ff0000 | ea);
			CODE_TRACE_OP(0, "MOVC", "A, @A + DPTR");
			CODE_TRACE_COMMENT("CODE[0x%08X (0x%02X + 0x%08X)] --> 0x%02X", ea, ACC, DPTR, res);
			ACC = res;
		}break;
		case 0x94: /* 2B */
			CODE_TRACE_OP(1, "SUBB", "A, #%02X", ARG(1)->v);
			ACC = _alu_box(vm, _sub_k);
		break;
		case 0xa2: /* 2B */
			SETUP_IR_BIT(ARG(1), 1);
			CODE_TRACE_OP(0, "MOV", "C, $%02X.%01u", ARG(1)->bit.ea, ARG(1)->bit.pos);
			BSET_AS(PSW, PSW_BIT_CY, !!(ARG(1)->v & ARG(1)->bit.bit));
		break;
		case 0xa3: /* 2C */
			CYCLE++;
			CODE_TRACE_OP(0, "INC", "DPTR");
			DPTR++;
			CODE_TRACE_COMMENT("0x%04X", DPTR);
		break;
		case 0xa8 ... 0xaf: /* 2B, 2C */
			CYCLE++;
			SETUP_IR_DIR(ARG(1), 1);
			CODE_TRACE_OP(1, "MOV", "R%01u, $%02X", IR_Rn, ARG(1)->ea);
			CODE_TRACE_COMMENT("0x02", ARG(1)->v);
			_IR_Rn_(vm) = ARG(1)->v;
		break;
		case 0xc0: /* 2B, 2C */
			CYCLE++;
			SETUP_IR_DIR(ARG(0), 1);
			CODE_TRACE_OP(1, "PUSH", "$%02X", ARG(0)->ea);
			push(vm, ARG(0)->v, 1);
			CODE_TRACE_COMMENT("(SP = 0x%04X), 0x02", SP, ARG(0)->v);
		break;
		case 0xc2: /* 2B */
			SETUP_IR_BIT(ARG(0), 1);
			CODE_TRACE_OP(1, "CLR", "$%02X.%01X", ARG(0)->bit.ea, ARG(0)->bit.pos);
			res = _alu_box(vm, _clr_k);
			direct_write(vm, ARG(0)->bit.ea, res);
		break;
		case 0xc3:
			CODE_TRACE_OP(0, "CLR", "C");
			BCLR(PSW, PSW_BIT_CY);
		break;
		case 0xc5:
			CODE_TRACE_OP(1, "XCH", "A, $%02X", ARG(1)->ea);
			CODE_TRACE_COMMENT("0x%02X <--> 0x%02x", ARG(0)->v, ARG(1)->v);
			ACC = ARG(1)->v;
			direct_write(vm, ARG(1)->ea, ARG(0)->v);
		break;
		case 0xc8 ... 0xcf:
			CODE_TRACE_OP(1, "XCH", "A, R%01u", IR_Rn);
			CODE_TRACE_COMMENT("0x%02X <--> 0x%02x", ARG(0)->v, ARG(1)->v);
			ACC = ARG(1)->v;
			_IR_Rn_(vm) = ARG(0)->v;
		break;
		case 0xd0: /* 2B, 2C */
			CYCLE++;
			SETUP_IR_DIR(ARG(0), 1);
			CODE_TRACE_OP(1, "POP", "$%02X", ARG(0)->ea);
			res = pop(vm, 1);
			CODE_TRACE_COMMENT("(SP = 0x%04X), 0x%02X", SP, res);
			direct_write(vm, ARG(0)->ea, res);
		break;
		case 0xd2: /* 2B */
			SETUP_IR_BIT(ARG(0), 1);
			CODE_TRACE_OP(1, "SETB", "$%02X.%01X", ARG(0)->bit.ea, ARG(0)->bit.pos);
			res = _alu_box(vm, _setb_k);
			direct_write(vm, ARG(0)->bit.ea, res);
		break;
		case 0xd8 ... 0xdf: /* 2B, 2C */
			CYCLE++;
			SETUP_IR_Rn_REL(ARG(0), ARG(1));
			CODE_TRACE_OP(1, "DJNZ", "R%01u, #%02X", IR_Rn, _RJMP(ARG(1)->ea));
			res = ARG(0)->v - 1;
			CODE_TRACE_COMMENT("(0x%04X - 1 --> 0x%04X) will%sjump",
				ARG(0)->v, res, res ? " " : " not ");
			_IR_Rn_(vm) = res;
			if(res)
				RJMP(ARG(1)->ea);
		break;
		case 0xe0: /* 2C */
			CYCLE++;
			CODE_TRACE_OP(0, "MOVX", "A, @DPTR");
			res = ld(vm, DPX);
			CODE_TRACE_COMMENT("0x%08X --> 0x%02X", DPX, res);
			ACC = res;
		break;
		case 0xe5: /* 2B */
			SETUP_IR_DIR(ARG(1), 1);
			CODE_TRACE_OP(0, "MOV", "A, $%02X", ARG(1)->ea);
			res = ARG(1)->v;
			CODE_TRACE_COMMENT("0x%02X", res);
			ACC = res;
		break;
		case 0xe4:
			CODE_TRACE_OP(0, "CLR", "A");
			ACC = 0;
		break;
		case 0xe6 ... 0xe7:
			CODE_TRACE_OP(0, "MOV", "A, @R%01u", IR_Ri);
			res = _IR_Ri_(vm);
			CODE_TRACE_COMMENT("0x%02X", res);
			ACC = res;
		break;
		case 0xe8 ... 0xef:
			CODE_TRACE_OP(0, "MOV", "A, R%01u", IR_Rn);
			res = _IR_Rn_(vm);
			CODE_TRACE_COMMENT("0x%02X", res);
			ACC = res;
		break;
		case 0xf0:
			CODE_TRACE_OP(0, "MOVX", "@DPTR, A");
			CODE_TRACE_COMMENT("0x%02X --> [0x%08X]", ACC, DPX);
			st(vm, DPX, ACC);
		break;
		case 0xf4:
			CODE_TRACE_OP(0, "CPL", "A");
			res = ~ACC;
			CODE_TRACE_COMMENT("0x%02X --> 0x%02X", ACC, res);
			ACC = res;
		break;
		case 0xf5: /* 2B */
			SETUP_IR_DIR(ARG(0), 0);
			CODE_TRACE_OP(1, "MOV", "$%02X, A", ARG(0)->ea);
			CODE_TRACE_COMMENT("0x%02X", ACC);
			direct_write(vm, ARG(0)->ea, ACC);
		break;
		case 0xf6 ... 0xf7:
			CODE_TRACE_OP(0, "MOV", "@R%01X, A", IR_Ri);
			CODE_TRACE_COMMENT("0x%02X", ACC);
			indirect_write(vm, IR_Ri, ACC);
		break;
		case 0xf8 ... 0xff:
			CODE_TRACE_OP(0, "MOV", "R%01u, A", IR_Rn);
			CODE_TRACE_COMMENT("0x%02X", ACC);
			_IR_Rn_(vm) = ACC;
		break;
		case 0xa569:
		break;
		default:
			switch(IR & 0x1f) {
				case 0x01:
				case 0x11: { /* 2B, 2C */
					CYCLE++;
					SETUP_IR_ADDR11(ARG(0));
					const int is_call = IR & 0x10;
					const char* acjs = is_call ? "LCALL" : "LJMP";
					CODE_TRACE_OP(1, acjs, "0x%08X", _JMP(ARG(0)->ea));
					if(is_call) {
						push(vm, PC, 2);
						CODE_TRACE_COMMENT("0x%04X", SP);
					}
					JMP(ARG(0)->ea);
				}break;
				default:
					CODE_TRACE_OP(0, " ???? ", " ???? ");
				break;
			};
		break;
	}
	
	CODE_TRACE_COMPLETE();
}

static uint16_t load_code_table_entry(vm_p vm, int entry)
{
	code_idx_ent_p code_table = (void*)vm->code_app;
	code_idx_ent_p code_entry = &code_table[entry];

	uint16_t len = bswap16(code_entry->len);
	uint16_t load_at = bswap16(code_entry->load_at);
	uint16_t offset = bswap16(code_entry->offset);
	
	TRACE("offset = 0x%04x, load_at = 0x%04x, length = 0x%04x",
		offset, load_at, len);
	
	uint8_t* src = &vm->code_app[offset];
	uint8_t* dst = &vm->xrom[load_at];
	for(int i = len + 1; i; i--)
		*dst++ = *src++;

	return(load_at);
}

static void* load_mmap(vm_p vm, const char* path, const char* mmats)
{
		int fd;
		struct stat sb;

		ON_1ERR(fail_open, fd = open(path, O_RDONLY));
		ON_1ERR(fail_fstat, fstat(fd, &sb));

		uint8_t* mmat;
		ON_ERR(fail_mmap, MAP_FAILED,
			mmat = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0));

		TRACE("%s = 0x%08x, size = 0x%08x",
			mmats, (int)mmat, (int)sb.st_size);

		close(fd);	fd = -1;

		return(mmat);

fail_mmap:
fail_fstat:
	if(-1 != fd)
		close(fd);
fail_open:
	return((void*)-1);
}

#include "vm_test.h"

int main(void)
{
	vm_p vm;
	
	ON_0ERR(fail_vm_alloc, vm = calloc(1, sizeof(vm_t)));

	vm->buddha_rom = load_mmap(vm, "archive/buddha_rom.bin", "buddha_rom");
	vm->code_app = load_mmap(vm, "archive/code.app", "code_app");
//	vm->flash_bin = load_mmap(vm, "archive/flash.bin", "flash_bin");

	if(1) {
		printf("     |   idx  |   len  |   ???? |  load@ |   ???? | offset |   dcrc |   tcrc |\n");

		code_idx_ent_p code_table = (void*)vm->code_app;

		for(int i = 0; i < bswap16(code_table[0].idx); i++)
		{
			code_idx_ent_p code_entry = &code_table[i];

			printf("0x%02x | 0x%04x | 0x%04x | 0x%04x | 0x%04x | 0x%04x | 0x%04x | 0x%04x | 0x%04x |\n",
				i, bswap16(code_entry->idx), bswap16(code_entry->len),
				bswap16(code_entry->unk1), bswap16(code_entry->load_at),
				bswap16(code_entry->unk2), bswap16(code_entry->offset),
				bswap16(code_entry->dcrc), bswap16(code_entry->tcrc));
		}
	}

	vm_reset(vm);

	memcpy(vm->irom, &vm->buddha_rom[0x8000], 0x1fff);
	memcpy(vm->xram, vm->buddha_rom, 65536);
	memcpy(vm->xrom, vm->buddha_rom, 65536);

	if(1) {
		load_code_table_entry(vm, 0);
//		load_code_table_entry(vm, 1);
		load_code_table_entry(vm, 2);
//		load_code_table_entry(vm, 3);

		JMP(0x2200);
	} else
		JMP(0x8000);

	for(int i = 0; i < 1024; i++)
		vm_test_step(vm);

	TRACE("CYCLE = 0x%016llx", CYCLE);

	if(vm)
		free(vm);

	return(0);

fail_vm_alloc:
	return(-1);
}
