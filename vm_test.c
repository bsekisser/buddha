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

typedef struct bit_t** bit_h;
typedef struct bit_t* bit_p;
typedef struct bit_t {
	uint8_t		arg;
	uint8_t		bit;
	uint8_t		ea;
	uint8_t		mask;
	uint8_t		pos;
}bit_t;

#define dst_bit(_x) \
	bit_p p2##_x; \
	_bit_ea(vm, &bit); \

static uint8_t _bit_ea(vm_p vm, bit_h h2bit)
{
	static bit_t bit, *p2bit = &bit;
	
	p2bit->arg = ld_ia(vm, &PC, 1);
	p2bit->pos = p2bit->arg & 7;
	p2bit->bit = 1 << p2bit->pos;
	p2bit->ea = p2bit->arg & 0xf8;
	p2bit->mask = ~p2bit->bit;
	
	*h2bit = p2bit;
	
	return(p2bit->ea);
}

#define dir_ea(_x) \
	const uint32_t _x##_ea = ld_ia(vm, &PC, 1);

/* **** */

#define dst_acc(_x)
#define dst_atDPTR(_x)
#define dst_atRi(_x)
#define dst_dir(_x)				dir_ea(_x);
#define dst_DPTR(_x)
#define dst_PSW_CY(_x)
#define dst_Rn(_x)

/* **** */

#define src_acc(_x) \
	const uint32_t _x = _src_acc(vm);

static uint32_t _src_acc(vm_p vm) {
	return(ACC);
}

#define src_addr11(_x) \
	const uint32_t _x = _src_addr11(vm);

static uint32_t _src_addr11(vm_p vm) {
	return((PC & ~_BM(11)) | ((IR >> 5) << 8) | ld_ia(vm, &PC, 1));
}

#define src_addr16(_x) \
	const uint32_t _x = _src_addr16(vm);

static uint32_t _src_addr16(vm_p vm) {
	return(ld_ia_be(vm, &PC, 2));
}

#define src_atA_DPTR(_x) \
	const uint32_t _x = _src_atA_DPTR(vm);

static uint32_t _src_atA_DPTR(vm_p vm) {
	return((ACC + DPTR));
}

#define src_atDPTR(_x) \
	const uint32_t _x = _src_atDPTR(vm);

static uint32_t _src_atDPTR(vm_p vm) {
	return(ld(vm, DPX));
}

#define src_atRi(_x) \
	const uint32_t _x = _src_atRi(vm);

static uint32_t _src_atRi(vm_p vm) {
	return(ld(vm, _IR_Ri_));
}

#define src_bit(_x) \
	bit_p p2##_x; \
	const uint32_t _x = _src_bit(vm, &p2##_x);
	
static uint32_t _src_bit(vm_p vm, bit_h h2bit) {
	return(ld(vm, _bit_ea(vm, h2bit)));
}

#define src_dir(_x) \
	dir_ea(_x); \
	const uint32_t _x = _src_dir(vm, _x##_ea); \

static uint32_t _src_dir(vm_p vm, uint32_t dir_ea) {
	return(ld(vm, dir_ea));
}

#define src_imm8(_x) \
	const uint32_t _x = _src_imm8(vm);

static uint32_t _src_imm8(vm_p vm) {
	return(ld_ia(vm, &PC, 1));
}

#define src_imm16(_x) \
	const uint32_t _x = _src_imm16(vm);

static uint32_t _src_imm16(vm_p vm) {
	return(ld_ia(vm, &PC, 2));
}

#define src_imm16be(_x) \
	const uint32_t _x = _src_imm16be(vm);

static uint32_t _src_imm16be(vm_p vm) {
	return(ld_ia_be(vm, &PC, 2));
}

#define src_Rn(_x) \
	const uint32_t _x = _src_Rn(vm);

static uint32_t _src_Rn(vm_p vm) {
	return(_IR_Rn_);
}

#define src_rel(_x) \
	const int32_t _x = _src_rel(vm);

static int32_t _src_rel(vm_p vm) {
	return(ld_ia(vm, &PC, 1));
}

#define ea_x2y2 \
	const uint32_t x2y2 = ld_ia(vm, &PC, 1);

#define _src_x2y2					((IR >> 2) & 3)
	_Rn_(X2y2)
#define _src_x2Y2					(IR & 3)

/* **** */

static void wb_acc(vm_p vm, uint32_t x) {
	ACC = x;
}

static void wb_atDPTR(vm_p vm, uint32_t x) {
	st(vm, DPX, x);
}

static void wb_atRi(vm_p vm, uint32_t x) {
	st(vm, _IR_Ri_, x);
}

#define wb_bit(_vm, _x) _wb_bit(_vm, p2bit, _x);
static void _wb_bit(vm_p vm, bit_p bit, uint32_t x)	{
	st(vm, bit->ea, x);
}

static void wb_dir(vm_p vm, uint32_t x) {
	st(vm, ARG(0)->ea, x);
}

static void wb_DPTR(vm_p vm, uint32_t x) {
	DPTR = (DPTR & ~0xffff) | (x & 0xffff);
}

static void wb_DPX(vm_p vm, uint32_t x) {
	wb_DPTR(vm, x);
}

static void wb_PSW_CY(vm_p vm, uint32_t x) {
	BSET_AS(PSW, PSW_BIT_CY, x);
}

static void wb_Rn(vm_p vm, uint32_t x) {
	_IR_Rn_ = x;
}

/* **** */

enum {
	_adc_k,
	_add_k,
	_and_k,
	_cmp16_k,
	_or_k,
	_sub_k,
	_xor_k,
};

static uint32_t _alu_box(vm_p vm, int esac, uint32_t x0, uint32_t x1, void (*wb)(vm_p, uint32_t))
{
	char *ops = "";
	uint32_t res = x0;
	
	switch(esac) {
		case	_adc_k:
			ops = "+";
			res += x1 + !!PSW_CY;
		break;
		case	_add_k:
			ops = "+";
			res += x1;
		break;
		case	_and_k:
			ops = "&";
			res &= x1;
		break;
		case	_or_k:
			ops = "&";
			res |= x1;
		break;
		case	_sub_k:
			ops = "-";
			res -= x1;
		break;
		case	_xor_k:
			ops = "^";
			res ^= x1;
	}

	if(0) CODE_TRACE_COMMENT("0x%02X %s 0x%02X --> 0x%02X", x0, x1, res);
	
	if(wb)
		wb(vm, res);

	return(res);
}
	

#define _alu_op(_x0, _op, _x1, _carry) \
	src_##_x0(_x0); \
	src_##_x1(_x1); \
	const uint32_t res = _alu_box(vm, _op, _x0, _x1, 0);

#define alu_op(_x0, _op, _x1, _wb) \
	({ \
		CODE_TRACE_COMMENT("0x%02X " #_op " 0x%02X --> 0x%02X", _x0, _x1, res); \
		if(_wb) \
			wb_##_x0(vm, res); \
	})

/* **** */

#define _acall(_addr11) _ajmp(_addr11);
#define acall(_addr11) \
	push(vm, PC, 2); \
	JMP(_addr11);

#define _ajmp(_addr11) \
	src_##_addr11(_addr11);
#define ajmp(_addr11) \
	JMP(_addr11);

#define _adc(_x0, _x1) _alu_op(_x0, _adc_k, _x1, 1);
#define adc(_x0, _x1) add(_x0, _x1);

#define _add(_x0, _x1) _alu_op(_x0, _add_k, _x1, 0);
#define add(_x0, _x1) \
	({ \
		alu_op(_x0, +, _x1, 1); \
		_add_set_flags(vm, _x0, _x1, res); \
	})

#define _anl(_x0, _x1) _alu_op(_x0, _and_k, _x1, 0);
#define anl(_x0, _x1) alu_op(_x0, &, _x1, 1);

#define _inc(_x) \
	src_##_x(_x); \
	const uint32_t res = _x + 1;
#define inc(_x) \
	({ \
		CODE_TRACE_COMMENT("0x%02X --> 0x%02X", _x, res); \
		wb_##_x(vm, res); \
	})

#define _clr(_x) dst_##_x(_x);
#define clr(_x) wb_##_x(vm, 0);

#define _clrb(_bit) \
	src_##_bit(_bit); \
	const uint32_t res = _bit & p2##_bit->mask;
#define clrb(_bit) \
	({ \
		CODE_TRACE_COMMENT("0x%02X & ~0x%02X --> 0x%02X", _bit , p2##_bit->bit, res); \
		wb_##_bit(vm, res); \
	})

#define _cmp16(_x0, _x1) _alu_op(_x0, _cmp16_k, _x1, 0);
#define cmp16(_x0, _x1) alu_op(_x0, -, _x1, 0);

#define _dec(_x) \
	src_##_x(_x); \
	const uint32_t res = _x - 1;
#define dec(_x) \
	({ \
		CODE_TRACE_COMMENT("0x%02X --> 0x%02X", _x, res); \
		wb_##_x(vm, res); \
	})

#define _djnz(_x, _rel) \
	src_##_x(_x); \
	_sjmp(_rel); \
	const uint32_t res = _x - 1;
#define djnz(_x, _rel) \
	({ \
		CODE_TRACE_COMMENT("(0x%04X - 1 --> 0x%04X) will%sjump", _x, res, res ? " " : " not "); \
		wb_##_x(vm, res); \
		if(res) \
			RJMP(_rel); \
	})

#define _jbc(_bit, _rel) \
	src_##_bit(_bit); \
	const uint32_t res = _bit & p2##_bit->mask; \
	_j_cc(_rel, !(_bit & p2##_bit->bit));
#define jbc(_bit, _rel) \
	wb_##_bit(vm, res); \
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

#define _lcall(_addr16) _ljmp(_addr16);
#define lcall(_addr16) \
	push(vm, PC, 2); \
	JMP(_addr16);

#define _ljmp(_addr16) \
	src_##_addr16(_addr16);
#define ljmp(_addr16) \
	JMP(_addr16);

#define _ljmp_i(_addr16) _ljmp(_addr16);
#define ljmp_i(_addr16) \
	({ \
		CODE_TRACE_COMMENT("0x%08X", _addr16); \
		ljmp(_addr16); \
	})

#define _mov(_x0, _x1) \
	dst_##_x0(_x0); \
	src_##_x1(_x1);
#define mov(_x0, _x1) \
	({ \
		CODE_TRACE_COMMENT("0x%02X", _x1); \
		wb_##_x0(vm, _x1); \
	})

#define _nop()
#define nop()

#define _orl(_x0, _x1) _alu_op(_x0, _or_k, _x1, 0);
#define orl(_x0, _x1) alu_op(_x0, |, _x1, 1);

#define __pop(_x) \
	dst_##_x(_x) \
	const uint32_t _x = pop(vm, 1);
#define _pop(_x) \
	({ \
		CODE_TRACE_COMMENT("(SP = 0x%04X), 0x%02X", SP, _x); \
		wb_##_x(vm, _x); \
	})

#define _push_(_x) src_##_x(_x);
#define push_(_x) \
	({ \
		push(vm, _x, 1); \
		CODE_TRACE_COMMENT("(SP = 0x%04X)", SP); \
	})

#define _ret()
#define ret() \
	({ \
		const uint32_t new_pc = pop(vm, 2); \
		CODE_TRACE_COMMENT("(SP = 0x%04X)", SP); \
		JMP(new_pc); \
	})

#define _setb(_bit) \
	src_##_bit(_bit); \
	const uint32_t res = _bit | p2##_bit->bit;
#define setb(_bit) alu_op(_bit, |, p2##_bit->bit, 1);

#define _sjmp(_rel) \
	src_##_rel(_rel);
#define sjmp(_rel) \
	RJMP(_rel)

#define _sub(_x0, _x1) _alu_op(_x0, _sub_k, _x1, 0)
#define sub(_x0, _x1) alu_op(_x0, -, _x1, 1)

#define _swap(_x) \
	src_##_x(_x); \
	const uint32_t res = ((_x & 0xf0f0f0f0) >> 4) | ((_x & 0x0f0f0f0f) << 4);
#define swap(_x) \
	({ \
		CODE_TRACE_COMMENT("0x%02X --> 0x%02X", _x, res); \
		wb_##_x(vm, res); \
	})

#define _xch(_x0, _x1) \
	src_##_x0(_x0); \
	src_##_x1(_x1);
#define xch(_x0, _x1) \
	({ \
		CODE_TRACE_COMMENT("0x%02X, 0x%02X --> 0x%02x, 0x%02X", _x0, _x1, _x1, _x0); \
		wb_##_x0(vm, _x1); \
		wb_##_x1(vm, _x0); \
	})

#define _xrl(_x0, _x1) _alu_op(_x0, _xor_k, _x1, 0)
#define xrl(_x0, _x1) alu_op(_x0, ^, _x1, 1)

/* **** */

#define INST_ESAC_LIST \
	INST(ESAC(0x00), 0, 0, nop(), "NOP", "") \
	INST(ESAC(0x02), 3, 2, ljmp(addr16), "LJMP", "0x%08X", _JMP(addr16)) \
	INST(ESAC(0x04), 0, 0, inc(acc), "INC", "A") \
	INST(ESAC(0x10), 3, 2, jbc(bit, rel), "JBC" , "$%02X, 0x%08X", p2bit->ea, p2bit->pos, _RJMP(rel)) \
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
	INST(ESAC(0xa2), 2, 2, mov(PSW_CY, bit), "MOV", "C, $%02X.%01u", p2bit->ea, p2bit->pos) \
	INST(ESAC(0xc0), 2, 2, push_(dir), "PUSH", "$%02X", dir_ea) \
	INST(ESAC(0xc2), 2, 0, clrb(bit), "CLR", "$%02X.%01u", p2bit->ea, p2bit->pos) \
	INST(ESAC(0xc3), 0, 0, clr(PSW_CY), "CLR", "C") \
	INST(ESAC(0xc4), 0, 0, swap(acc), "SWAP", "A") \
	INST(ESAC(0xd0), 2, 2, _pop(dir), "POP", "$%02X", dir_ea) \
	INST(ESAC(0xd2), 2, 0, setb(bit), "SETB", "$%02X.%01u", p2bit->ea, p2bit->pos) \
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
	INST_ESAC(0xa560, 0xfff0, 2, 0, cmp16(X2y2, x2Y2), "CMP16", "%02X, %02X", X2y2, x2Y2)

void vm_test_a5_inst(vm_p vm)
{
	INST_ESAC_LIST_a5
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
