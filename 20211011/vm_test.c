#include <assert.h>
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

#include "vm8051_trace.h"

extern char spaces[61];

#define _JMP(_ea) (~0xffffUL + (_ea & 0xffffUL))
#define JMP(_ea) PC = _JMP(_ea);

#define _RJMP(_rel) (PC + (int8_t)_rel)
#define RJMP(_rel) PC = _RJMP((int8_t)_rel);

#include "on_err.h"
#include "vm8051.h"
#include "vm_test.h"
//#include "vm8051_ea.h"
//#include "vm8051_ir.h"


//#define atIP(_ofs) ld(vm, IP + _ofs)

/* **** */

#define _Rn_(_r) \
	vm->iram[(PSW & (3 << 3)) | (_r & 7)]

//#define IR_Ri						IR & 1
//#define IR_Rn						IR & 7

#define _IR_Ri_						_Rn_(IR_Ri)
#define _IR_Rn_						_Rn_(IR_Rn)

/* **** */

#define	BIT_EA						(bit_arg & 0xf8)
#define BIT_POS						(bit_arg & 7)

/* **** */

/*typedef struct arg_t* arg_p;
typedef struct arg_t {
	uint8_t type;
	uint32_t arg;
	uint32_t v;
}arg_t;*/

#undef ARG_ACTION
#define ARG_ACTION(_esac, _action) \
	case _arg_t_##_esac: \
		_action; \
	break;

#undef ARG_ESAC
#define ARG_ESAC(_esac, _arg) \
	ARG_ACTION(_esac, x->arg = _arg)

enum {
	_arg_t_nop,
	_arg_t_acc,
	_arg_t_addr11,
	_arg_t_addr16,
	_arg_t_atA_DPTR,
	_arg_t_atDPTR,
	_arg_t_atRi,
	_arg_t_bit,
	_arg_t_dir,
	_arg_t_DPTR,
	_arg_t_DPX,
	_arg_t_imm8,
	_arg_t_imm16,
	_arg_t_imm16be,
	_arg_t_PSW_CY,
	_arg_t_rel,
	_arg_t_Rn,
};

#define arg(_x) _arg_t_##_x
static arg_p arg_arg(vm_p vm, int type)
{
	arg_p x = ARG(IXR->argc++);
	x->type = type;
	
	switch(type) {
		ARG_ESAC(addr11, ld_ia(vm, &PC, 1));
		ARG_ESAC(addr16, ld_ia_be(vm, &PC, 2));
		ARG_ESAC(atA_DPTR, (ACC + DPTR));
		ARG_ESAC(atDPTR, DPTR);
		ARG_ESAC(atRi, IR_Ri);
		ARG_ESAC(bit, ld_ia(vm, &PC, 1));
		ARG_ESAC(dir, ld_ia(vm, &PC, 1));
		ARG_ESAC(rel, ld_ia(vm, &PC, 1));
	}
}

static void dst_arg(vm_p vm, arg_p x)
{
	switch(x->type) {
		ARG_ACTION(bit, arg_arg(vm, x));
		ARG_ACTION(dir, arg_arg(vm, x));
	}
}

#undef ARG_ESAC
#define ARG_ESAC(_esac, _arg) \
	ARG_ACTION(_esac, x->v = _arg)

static void src_arg(vm_p vm, arg_p x)
{
	arg_arg(vm, x);
	
	switch(x->type) {
		ARG_ESAC(acc, ACC);
		ARG_ESAC(addr11, ((PC & ~_BM(11)) | ((IR >> 5) << 8) | x->arg));
		ARG_ESAC(addr16, x->arg);
		ARG_ESAC(atA_DPTR, ld(vm, x->arg));
		ARG_ESAC(atDPTR, ld(vm, x->arg));
		ARG_ESAC(atRi, ld(vm, x->arg));
		ARG_ESAC(bit, bit_read(vm, x->arg));
		ARG_ESAC(dir, ld(vm, x->arg));
		ARG_ESAC(imm8, ld_ia(vm, &PC, 1));
		ARG_ESAC(imm16, ld_ia(vm, &PC, 2));
		ARG_ESAC(imm16be, ld_ia_be(vm, &PC, 2));
		ARG_ESAC(Rn, _IR_Rn_);
	}
}

#define ea_x2y2 \
	const uint32_t x2y2 = ld_ia(vm, &PC, 1);

//#define _src_x2y2					((IR >> 2) & 3)
//	_Rn_(X2y2)
//#define _src_x2Y2					(IR & 3)

/* **** */

static void wb_arg(vm_p vm, arg_p x, uint32_t v)
{
	switch(x->type) {
		ARG_ACTION(acc, ACC = v);
		ARG_ACTION(atDPTR, st(vm, x->arg, v));
		ARG_ACTION(atRi, st(vm, x->arg, v));
		ARG_ACTION(bit, bit_write(vm, x->arg, v));
		ARG_ACTION(dir, st(vm, x->arg, v));
		ARG_ACTION(DPTR, DPTR = (DPTR & ~0xffff) | (v & 0xffff));
		ARG_ACTION(DPX, DPTR = (DPTR & ~0xffff) | (v & 0xffff));
		ARG_ACTION(PSW_CY, BSET_AS(PSW, PSW_BIT_CY, v));
		ARG_ACTION(Rn, _IR_Rn_ = v);
	}
}

/* **** */

enum {
	_nop_k,
	_adc_k,
	_add_k,
	_and_k,
	_cmp16_k,
	_or_k,
	_mov_k,
	_sub_k,
	_xor_k,

	/* **** */

	_anl_k = _and_k,
	_orl_k = _or_k,
	_xrl_l = _xor_k,
};

#define alu_box(_x0, _esac, _x1) _alu_box(vm, stage, _esac, arg(_x0), arg(_x1))
static void _alu_box(vm_p vm, int stage, int esac, arg_h x0, arg_h x1)
{
	if(!stage) {
		WB = 1;
		RES = x0->v;

		switch(esac) {
			case	_adc_k:
				RES += (x1->v + !!PSW_CY);
			break;
			case	_add_k:
				RES += x1->v;
			break;
			case	_and_k:
				RES &= x1->v;
			break;
			case	_mov_k;
				RES = x1->v;
			break;
			case	_or_k:
				RES |= x1->v;
			break;
			case	_cmp16_k:
				wb = 0;
			case	_sub_k:
				RES -= x1->v;
			break;
			case	_xor_k:
				RES ^= x1->v;
			break;
		}
	} else {
		CODE_TRACE_COMMENT("0x%02X %s 0x%02X --> 0x%02X", x0, x1, RES);
	
		if(wb)
			wb_arg(vm, x0, RES);
	}
}
	
/* **** */

#define add(_x0, _x1) alu_box(_x0, _add_t, _x1)
#define adc(_x0, _x1) alu_box(_x0, _adc_t, _x1)

#static ajmp(_addr11) _ajmp(vm, stage, arg(_addr11)
static void _ajmp(vm_p vm, int stage, arg_p addr11)
{
	if(!stage) {
		src_arg(vm, addr11);
	} else {
		if(IR & 0x10) {
			push(vm, PC, 2);
			CODE_TRACE_COMMENT("(SP = 0x%02X) 0x%08X", SP, addr11->v);
		} else {
			CODE_TRACE_COMMENT("0x%08X", addr11->v);
		}

		JMP(addr11->v);
	}
}

#define inc(_x) alu_box(_x, inc_t, implied_v(1))

#define clr(_x) _clr(vm, stage, arg(_x))
static void _clr(vm_p vm, int stage, arg_p x)
{
	if(!stage)
		dst_arg(vm, x);
	else
		wb_arg(vm, x, 0);
}

static void cmp16(vm_p vm, int stage, arg_p xy)
{
//	#define _cmp16(_x0, _x1) _alu_op(_x0, _cmp16_k, _x1, 0);
//	#define cmp16(_x0, _x1) alu_op(_x0, -, _x1, 0);
}

#define dec(_x) alu_box(_x, dec_t, implied_v(1))

#define djnz(_x, _rel) _djnz(vm, stage, arg(_x), arg(_rel))
static void _djnz(vm_p vm, int stage, arg_p x, arg_p rel)
{
	if(!stage) {
		dst_arg(x);
		src_arg(rel);
		RES = x->v - 1;
	} else
		CODE_TRACE_COMMENT("(0x%04X - 1 --> 0x%04X) will%sjump",
			x->v, RES, RES ? " " : " not ");

		wb_arg(vm, x, res)
		if(RES)
			RJMP(rel);
	}
}

#define _jbc(_bit, _rel) \
	dst_src(_bit); \
	_j_cc(_rel, 0 == _bit);
#define jbc(_bit, _rel) \
	wb_##_bit(vm, 0); \
	j_cc(_rel);

#define _j_cc(_rel, _test) \
	_sjmp(_rel); \
	const uint32_t _test_res = (_test);

#define j_cc(_rel) \
	({ \
		CODE_TRACE_COMMENT("will%sjump", _test_res ? " " : " not "); \
		if(_test_res) \
			RJMP(_rel); \
	})

#define _jnc(_rel) _j_cc(_rel, !(!!PSW_CY));
#define jnc(_rel) j_cc(_rel);

#define _jnz(_rel) _j_cc(_rel, 0 != ACC);
#define jnz(_rel) j_cc(_rel);

#define _jz(_rel) _j_cc(_rel, 0 == ACC);
#define jz(_rel) j_cc(_rel);

#define lcall ljmp

#define ljmp(_addr16) _ljmp(vm, stage, arg(_addr16))
static void _ljmp(vm_p vm, int stage, arg_p addr16)
{
	if(!stage) {
		src_arg(vm, addr16);
	} else {
		if(IR & 0x10) {
			push(vm, PC, 2);
			CODE_TRACE_COMMENT("(SP = 0x%02X) 0x%08X", SP, addr16->v);
		} else {
			CODE_TRACE_COMMENT("0x%08X", addr16->v);
		}

		JMP(addr16);
	}
}

#define mov(_x0, _x1) alu_box(_x0, _mov_k, _x1)

static void nop(vm_p vm, int stage)
{
}

#define orl(_x0, _x1) alu_box(_x0, _orl_t, _x1)

static void _pop(vm_p vm, int stage, arg_p x)
{
	if(!stage) {
		dst_arg(vm, x);
		RES = pop(vm, 1);
	} else {
		CODE_TRACE_COMMENT("(SP = 0x%04X), 0x%02X", SP, RES);
		wb_arg(vm, x, RES);
	}
}

static void _push(vm_p vm, int stage, arg_p x)
{
	if(stage) {
		push(vm, x->v, 1);
		CODE_TRACE_COMMENT("(SP = 0x%04X)", SP);
	}
}

static void ret(vm_p vm, int stage)
{
	if(stage) {
		const uint32_t new_pc = pop(vm, 2);
		CODE_TRACE_COMMENT("(SP = 0x%04X)", SP);
		JMP(new_pc);
	}
}

#define setb(_bit) _setb(vm, stage, _bit)
static void setb(vm_p vm, int stage, arg_p bit)
{
	if(!stage) {
		dst_arg(vm, bit);
	} else
		wb_arg(vm, bit, 1);
}

static void sjmp(vm_p vm, int stage, arg_p rel)
{
	if(!stage) {
		src_arg(vm, rel);
	} else {
		RJMP(rel->v);
	}
}

#define sub(_x0, _x1) alu_op(_x0, _sub_k, _x1)

static void swap(vm_p vm, int stage, arg_p x)
{
	if(!stage) {
		src_arg(vm, x);
		RES = ((x->v & 0xf0f0f0f0) >> 4) | ((x->v & 0x0f0f0f0f) << 4);
	} else {
		CODE_TRACE_COMMENT("0x%02X --> 0x%02X", x->v, RES);
		wb_arg(vm, x, res);
	}
}

static void xch(vm_p vm, int stage, arg_p x0, arg_p x1)
{
	if(!stage) {
		src_arg(vm, x0);
		src_arg(vm, x1);
	} else {
		CODE_TRACE_COMMENT("0x%02X, 0x%02X --> 0x%02x, 0x%02X",
			x0->v, x1->v, x1-v, x0-v);

		wb_arg(vm, x0, x1->v);
		wb_arg(vm, x1, x0->v);
	}
}

#define xrl(_x0, _x1) alu_op(_x0, _xor_k, _x1)

/* **** */

#define INST_ESAC_LIST \
	INST(ESAC(0x00), 0, 0, nop(), "NOP", "") \
	INST(ESAC(0x02), 3, 2, ljmp(addr16), "LJMP", "0x%08X", _JMP(addr16)) \
	INST(ESAC(0x04), 0, 0, inc(acc), "INC", "A") \
	INST(ESAC(0x10), 3, 2, jbc(bit, rel), "JBC" , "$%02X, 0x%08X", BIT_EA, BIT_POS, _RJMP(rel)) \
	INST(ESAC(0x12), 3, 2, lcall(addr16), "LCALL", "0x%08X", _JMP(addr16)) \
	INST(ESAC(0x22), 0, 2, ret(), "RET", "") \
	INST(ESAC(0x42), 2, 0, orl(dir, acc), "ORL", "$%02X, A", dir_ea) \
	INST(ESAC(0x43), 3, 2, orl(dir, imm8), "ORL", "$%02X, #%02X", dir_ea, imm8) \
	INST(ESAC(0x50), 2, 2, jnc(rel), "JNC", "0x%08X", _RJMP(rel)) \
	INST(ESAC(0x53), 3, 2, anl(dir, imm8), "ANL", "$%02X, #%02X", dir_ea, imm8) \
	INST(ESAC(0x54), 2, 0, anl(acc, imm8), "ANL", "A, #%02X", imm8) \
	INST(ESAC(0x60), 2, 2, jz(rel), "JZ", "0x%08X", _RJMP(rel)) \
	INST(ESAC(0x70), 2, 2, jnz(rel), "JNZ", "0x%08X", _RJMP(rel)) \
	INST(ESAC(0x73), 0, 2, ljmp_i(atA_DPTR), "JMP", "@A + DPTR") \
	INST(ESAC(0x74), 2, 0, mov(acc, imm8), "MOV", "A, #%02X", imm8) \
	INST(ESAC(0x80), 2, 2, sjmp(rel), "SJMP", "0x%08X", _RJMP(rel)) \
	INST(ESAC(0x90), 3, 2, mov(DPTR, imm16be), "MOV", "DPTR, #%04X", imm16be) \
	INST(ESAC(0xa2), 2, 2, mov(PSW_CY, bit), "MOV", "C, $%02X.%01u", BIT_EA, BIT_POS) \
	INST(ESAC(0xc0), 2, 2, push_(dir), "PUSH", "$%02X", dir_ea) \
	INST(ESAC(0xc2), 2, 0, clrb(bit), "CLR", "$%02X.%01u", BIT_EA, BIT_POS) \
	INST(ESAC(0xc3), 0, 0, clr(PSW_CY), "CLR", "C") \
	INST(ESAC(0xc4), 0, 0, swap(acc), "SWAP", "A") \
	INST(ESAC(0xd0), 2, 2, _pop(dir), "POP", "$%02X", dir_ea) \
	INST(ESAC(0xd2), 2, 0, setb(bit), "SETB", "$%02X.%01u", BIT_EA, BIT_POS) \
	INST(ESAC(0xe0), 0, 2, mov(acc, atDPTR), "MOVX", "A, @DPTR") \
	INST(ESAC(0xe4), 0, 0, clr(acc), "CLR", "A") \
	INST(ESAC(0xf0), 0, 2, mov(atDPTR, acc), "MOVX", "@DPTR, A") \
	INST_ESAC_LIST_Rx5 \
	INST_ESAC_LIST_Rx67 \
	INST_ESAC_LIST_Rx8F

#define INST_ESAC_LIST_Rx5 \
	INST(ESAC(0x05), 2, 0, inc(dir), "INC", "$%02X", dir_ea) \
	INST(ESAC(0x15), 2, 0, dec(dir), "DEC", "$%02X", dir_ea) \
	INST(ESAC(0x25), 2, 0, add(acc, dir), "ADD", "A, $%02X", dir_ea) \
	INST(ESAC(0x35), 2, 0, adc(acc, dir), "ADDC", "A, $%02X", dir_ea) \
	INST(ESAC(0x45), 2, 0, orl(acc, dir), "ORL", "A, $%02X", dir_ea) \
	INST(ESAC(0x55), 2, 0, anl(acc, dir), "ANL", "A, $%02X", dir_ea) \
	INST(ESAC(0x65), 2, 0, xrl(acc, dir), "XRL", "A, $%02X", dir_ea) \
	INST(ESAC(0x75), 3, 2, mov(dir, imm8), "MOV", "$%02X, #%02X", dir_ea, imm8) \
	INST(ESAC(0x95), 2, 0, sub(acc, dir), "SUBB", "A, $%02X", dir_ea) \
	INST(ESAC(0xc5), 2, 0, xch(acc, dir), "XCH", "A, $%02X", dir_ea) \
	INST(ESAC(0xd5), 3, 2, djnz(dir, rel), "DJNZ", "$%02X, 0x%08X", dir_ea, _RJMP(rel)) \
	INST(ESAC(0xe5), 2, 0, mov(acc, dir), "MOV", "A, $%02X", dir_ea) \
	INST(ESAC(0xf5), 2, 0, mov(dir, acc), "MOV", "$%02X, A", dir_ea, acc)
//	INST(ESAC(0x85), 3, 2, mov(dir0, dir), "MOV", "$%02X, $%02X", dir0_ea, dir_ea)

#define INST_ESAC_LIST_Rx67 \
	INST(ESAC_RANGE(0x06, 0x07), 0, 0, inc(atRi), "INC", "@R%01u", IR_Ri) \
	INST(ESAC_RANGE(0x16, 0x17), 0, 0, dec(atRi), "DEC", "@R%01u", IR_Ri) \
	INST(ESAC_RANGE(0x26, 0x27), 0, 0, add(acc, atRi), "ADD", "A, @R%01u", IR_Ri) \
	INST(ESAC_RANGE(0x36, 0x37), 0, 0, adc(acc, atRi), "ADDC", "A, @R%01u", IR_Ri) \
	INST(ESAC_RANGE(0x46, 0x47), 0, 0, orl(acc, atRi), "ORL", "A, @R%01u", IR_Ri) \
	INST(ESAC_RANGE(0x56, 0x57), 0, 0, anl(acc, atRi), "ANL", "A, @R%01u", IR_Ri) \
	INST(ESAC_RANGE(0x66, 0x67), 0, 0, xrl(acc, atRi), "XRL", "A, @R%01u", IR_Ri) \
	INST(ESAC_RANGE(0x86, 0x87), 2, 2, mov(dir, atRi), "MOV", "$%02X, @R%01u", dir_ea, IR_Ri) \
	INST(ESAC_RANGE(0x96, 0x97), 0, 0, sub(acc, atRi), "SUBB", "A, @R%01u", IR_Ri) \
	INST(ESAC_RANGE(0xa6, 0xa7), 2, 2, mov(atRi, dir), "MOV", "@R%01u, #%02X", IR_Ri, dir_ea) \
	INST(ESAC_RANGE(0xc6, 0xc7), 0, 0, xch(acc, atRi), "XCH", "A, @R%01u", IR_Ri) \
	INST(ESAC_RANGE(0xe6, 0xe7), 0, 0, mov(acc, atRi), "MOV", "A, @R%01u", IR_Ri) \
	INST(ESAC_RANGE(0xf6, 0xf7), 0, 0, mov(atRi, acc), "MOV", "@R%01u, A", IR_Ri)

#define INST_ESAC_LIST_Rx8F \
	INST(ESAC_RANGE(0x08, 0x0f), 0, 0, inc(Rn), "INC", "R%01u", IR_Rn) \
	INST(ESAC_RANGE(0x18, 0x1f), 0, 0, dec(Rn), "DEC", "R%01u", IR_Rn) \
	INST(ESAC_RANGE(0x28, 0x2f), 0, 0, add(acc, Rn), "ADD", "A, R%01u", IR_Rn) \
	INST(ESAC_RANGE(0x38, 0x3f), 0, 0, adc(acc, Rn), "ADDC", "A, R%01u", IR_Rn) \
	INST(ESAC_RANGE(0x48, 0x4f), 0, 0, orl(acc, Rn), "ORL", "A, R%01u", IR_Rn) \
	INST(ESAC_RANGE(0x58, 0x5f), 0, 0, anl(acc, Rn), "ANL", "A, R%01u", IR_Rn) \
	INST(ESAC_RANGE(0x68, 0x6f), 0, 0, xrl(acc, Rn), "XRL", "A, R%01u", IR_Rn) \
	INST(ESAC_RANGE(0x78, 0x7f), 2, 0, mov(Rn, imm8), "MOV", "R%01u, #%02X", IR_Rn, imm8) \
	INST(ESAC_RANGE(0x88, 0x8f), 2, 2, mov(dir, Rn), "MOV", "$%02X, R%01u", dir_ea, IR_Rn) \
	INST(ESAC_RANGE(0x98, 0x9f), 0, 0, sub(acc, Rn), "SUBB", "A, R%01u", IR_Rn) \
	INST(ESAC_RANGE(0xa8, 0xaf), 2, 2, mov(Rn, dir), "MOV", "R%01u, $%02X", IR_Rn, dir_ea) \
	INST(ESAC_RANGE(0xc8, 0xcf), 0, 0, xch(acc, Rn), "XCH", "A, R%01u", IR_Rn) \
	INST(ESAC_RANGE(0xd8, 0xdf), 2, 2, djnz(Rn, rel), "DJNZ", "R%01u, #%02X", IR_Rn, _RJMP(rel)) \
	INST(ESAC_RANGE(0xe8, 0xef), 0, 0, mov(acc, Rn), "MOV", "A, R%01u", IR_Rn) \
	INST(ESAC_RANGE(0xf8, 0xff), 0, 0, mov(Rn, acc), "MOV", "R%01u, A", IR_Rn)

#define INST_ESAC_LIST_MASKED \
	INST(MASKED(0x01, 0x1f), 2, 2, ajmp(addr11), "AJMP", "0x%08X", _JMP(addr11)) \
	INST(MASKED(0x11, 0x1f), 2, 2, acall(addr11), "ACALL", "0x%08X", _JMP(addr11))

/* **** */

#define INST(_esac, _bytes, _cycles, _action, _ops, _args, _varargs...) \
	static void _esac(vm_p vm) \
	{ \
		_##_action; \
		const uint8_t pcip_bytes = PC - IP; \
		CODE_TRACE_OP((pcip_bytes - 1), _ops, _args, ##_varargs); \
		if((_bytes ? _bytes : 1) != pcip_bytes) { \
			printf("IP = 0x%04x, PC = 0x%04x, cycles = 0x%04x, bytes = (0x%04x, expected 0x%04x)\n", \
			IP, PC, _cycles, pcip_bytes, _bytes); \
		} \
		_action; \
		if(_cycles) \
			CYCLE += _cycles - 1; \
	}

#define ESAC(_op) inst_##_op
#define ESAC_RANGE(_op, _range) ESAC(_op)
#define MASKED(_op, _mask) ESAC(_op)

INST_ESAC_LIST
INST_ESAC_LIST_MASKED

#undef ESAC
#define ESAC(_op) \
	case _op: { \
		inst_##_op(vm); \
	}break;

#undef ESAC_RANGE
#define ESAC_RANGE(_op, _range) \
	case _op ... _range: { \
		inst_##_op(vm); \
	}break;

#undef INST
#define INST(_esac, _bytes, _cycles, _action, _ops, _args, _varargs...) \
	_esac

#undef MASKED
#define MASKED(_op, _mask) \
	if(_op == (IR & _mask)) { \
		inst_##_op(vm); \
	}else


#define INST_ESAC_LIST_a5 \
	INST(MASKED(0xa560, 0xfff0), 2, 0, cmp16(X2y2, x2Y2), "CMP16", "%02X, %02X", X2y2, x2Y2)

void vm_test_a5_inst(vm_p vm)
{
//	INST_ESAC_LIST_a5
}

void vm_test_step(vm_p vm)
{
	IP = PC;
	
	IR = ld_ia(vm, &PC, 1);
		
	CODE_TRACE_START();

	CYCLE++;

	switch(IR) {
		default:
			if(0xa5 == IR)
				vm_test_a5_inst(vm);
			else 
			INST_ESAC_LIST_MASKED
			{
				CODE_TRACE_OP(6, "????", "????");
				CODE_TRACE_COMPLETE();
				exit(-1);
			}
		break;
		INST_ESAC_LIST
	}

	CODE_TRACE_COMPLETE();
}
