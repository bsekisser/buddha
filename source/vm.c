#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitfield.h"

#include "vm.h"

#include "arg.h"
#include "ldst.h"
#include "psw.h"
#include "sfr.h"
#include "stack.h"
#include "trace.h"

#include "inst_esac_list.h"

/* **** */

enum {
	_nop_k,
	_adc_k,
	_add_k,
	_and_k,
	_dec_k,
	_inc_k,
	_or_k,
	_mov_k,
	_movc_k,
	_movx_k,
	_sub_k,
	_xor_k,

	/* **** a5 -- 16 bit operattions*/

	_adc16_k,
	_add16_k,
	_cmp16_k,
	_dec16_k,
	_inc16_k,
	_sub16_k,

	/* **** alias */

	_addc16_k = _adc16_k,
	_addc_k = _adc_k,
	_anl_k = _and_k,
	_orl_k = _or_k,
	_subb_k = _sub_k,
	_xrl_k = _xor_k,
};

static void _alu_flags_add(vm_p vm, uint32_t x0, uint32_t x1, uint32_t res, int x16)
{
	const uint8_t co_bit = x16 ? 15 : 7;
	
	/* carry & half carry */
	uint32_t carry = (x0 & x1) | (x1 & ~res) | (~res & x0);
	BSET_AS(PSW, PSW_BIT_CY, (carry >> co_bit) & 1);
	BSET_AS(PSW, PSW_BIT_AC, (carry >> 3) & 1); /* x16 ?? */

	/* overflow */
	BSET_AS(PSW, PSW_BIT_OV, !!((((x0 & x1 & ~res) | (~x0 & ~x1 & res)) >> co_bit) & 1));
}

static void _alu_flags_sub(vm_p vm, uint32_t x0, uint32_t x1, uint32_t res, int x16)
{
	return(_alu_flags_add(vm, x0, -x1, res, x16));
}

#define alu_box(_x0, _esac, _x1) _alu_box(vm, _esac, ARG_T(_x0), ARG_T(_x1), 0)
#define alu16_box(_x0, _esac, _x1) _alu_box(vm, _esac, ARG_T(_x0), ARG_T(_x1), 1)

static void _alu_box(vm_p vm, int esac, arg_type x0, arg_type x1, int x16)
{
	arg_p x[2] = { arg_src(vm, x0), arg_src(vm, x1) };

	const char* ops;
	int wb = 1;
	RES = x[0]->v;

	uint32_t x1v = x1 ? x[1]->v : 1 << ((_inc16_k == esac) && IR & 4);

	switch(esac) {
		case	_adc16_k:
		case	_adc_k:
		case	_add16_k:
		case	_add_k:
			ops = "+";
			RES += (x[1]->v + ((_adc_k == esac) ? !!PSW_CY : 0));
			_alu_flags_add(vm, x[0]->v, x[1]->v, RES, 0);
		break;
		case	_and_k:
			ops = "&";
			RES &= x[1]->v;
		break;
		case	_dec16_k:
		case	_dec_k:
			ops = "-";
			RES -= x1v;
		break;
		case	_inc16_k:
		case	_inc_k:
			ops = "+";
			RES += x1v;
		break;
/*		case	_mov_k:
			ops = "--";
			RES = x[1]->v;
		break;
*/
		case	_or_k:
			ops = "|";
			RES |= x[1]->v;
		break;
		case	_cmp16_k:
			wb = 0;
			ops = "-";
			RES -= x[1]->v;
			_alu_flags_sub(vm, x[0]->v, x[1]->v, RES, 1);
		break;
		case	_sub16_k:
		case	_sub_k:
			ops = "-";
			RES -= (!!PSW_CY + x[1]->v);
			_alu_flags_sub(vm, x[0]->v, x[1]->v, RES, x16);
		break;
		case	_xor_k:
			ops = "^";
			RES ^= x[1]->v;
		break;
		default:
			TRACE("esac = 0x%02X", esac);
			exit(-1);
		break;
	}

	if(x16) {
		CODE_TRACE_COMMENT("0x%04X %s 0x%04X --> 0x%04X, CY = %X", x[0]->v & 0xffff, ops, x1v & 0xffff, RES & 0xffff, !!PSW_CY);
	} else {
		CODE_TRACE_COMMENT("0x%02X %s 0x%02X --> 0x%02X", x[0]->v & 0xff, ops, x1v & 0xff, RES & 0xff);
	}

	if(wb)
		arg_wb(vm, x[0], RES);
}

/* **** */


#define acall(_addr11) _ajmp(vm, ARG_T(_addr11))
#define ajmp(_addr11) _ajmp(vm, ARG_T(_addr11))
static void _ajmp(vm_p vm, arg_type _addr11)
{
	arg_p addr11 = arg_dst(vm, _addr11);

	if(0x11 == (IR & 0x1f)) {
		push(vm, PC, 2);
		CODE_TRACE_COMMENT("(SP = 0x%04X) 0x%08X", SP, addr11->v);
	} else {
		CODE_TRACE_COMMENT("0x%08X", addr11->v);
	}

	JMP(addr11->v);
}

#define add16(_x0, _x1) alu16_box(_x0, _add16_k, _x1)
#define add(_x0, _x1) alu_box(_x0, _add_k, _x1)
#define addc16(_x0, _x1) alu16_box(_x0, _addc16_k, _x1)
#define addc(_x0, _x1) alu_box(_x0, _addc_k, _x1)

#define anl(_x0, _x1) alu_box(_x0, _and_k, _x1)

#define clr(_x) _clr(vm, ARG_T(_x))
static void _clr(vm_p vm, arg_type _x)
{
	arg_p x = arg_dst(vm, _x);

//	printf("_x = 0x%08x, x = 0x%08x, x->arg = 0x%08x\n", _x, x, x->arg);

	arg_wb(vm, x, 0);
}

#define cmp16(_x0, _x1) alu16_box(_x0, _cmp16_k, _x1)

#define dec16(_x)					alu16_box(_x, _dec16_k, nop)

#define djnz(_x, _rel) _djnz(vm, ARG_T(_x), ARG_T(_rel))
static void _djnz(vm_p vm, arg_type _x, arg_type _rel)
{
	arg_p x = arg_src(vm, _x);
	arg_p rel = arg_dst(vm, _rel);
	RES = x->v - 1;

	CODE_TRACE_COMMENT("(0x%04X - 1 --> 0x%04X) will%sjump",
		x->v, RES, RES ? " " : " not ");

	arg_wb(vm, x, RES);
	if(RES)
		RJMP(rel->arg);
}

#define inc(_x)						alu_box(_x, _inc_k, nop)
#define inc16(_x)					alu16_box(_x, _inc16_k, nop)

#define jb(_bit, _rel)				_jb_wbc(vm, ARG_T(_bit), ARG_T(_rel), 0)
#define jbc(_bit, _rel)				_jb_wbc(vm, ARG_T(_bit), ARG_T(_rel), 1)
static void _jb_wbc(vm_p vm, arg_type _bit, arg_type _rel, int wbc)
{
	arg_p bit = arg_src(vm, _bit);
	arg_p rel = arg_dst(vm, _rel);

	const uint32_t _test_res = !!bit->v;

	CODE_TRACE_COMMENT("will%sjump", _test_res ? " " : " not ");

	if(_test_res)
	{
		if(wbc)
			arg_wb(vm, bit, 0);

		RJMP(rel->arg);
	}
}

#define jc(_rel) j_cc(_rel, !!PSW_CY)

#define j_cc(_rel, _test) _j_cc(vm, ARG_T(_rel), _test)
static void _j_cc(vm_p vm, arg_type _rel, const uint32_t _test_res)
{
	arg_p rel = arg_dst(vm, _rel);

	if(IR != 0x80)
		CODE_TRACE_COMMENT("will%sjump", _test_res ? " " : " not ");

	if(_test_res)
		RJMP(rel->arg);
}

#define jnb(_bit, _rel) _jnb(vm, ARG_T(_bit), ARG_T(_rel))
static void _jnb(vm_p vm, arg_type _bit, arg_type _rel)
{
	arg_p bit = arg_src(vm, _bit);
	arg_p rel = arg_dst(vm, _rel);

	const uint32_t _test_res = !(!!bit->v);

	CODE_TRACE_COMMENT("will%sjump", _test_res ? " " : " not ");

	if(_test_res)
		RJMP(rel->arg);
}

#define jnc(_rel) j_cc(_rel, !(!!PSW_CY))
#define jnz(_rel) j_cc(_rel, 0 != ACC);
#define jz(_rel) j_cc(_rel, 0 == ACC)

#define lcall ljmp
#define ljmp(_addr16) _ljmp(vm, ARG_T(_addr16))
static void _ljmp(vm_p vm, arg_type _addr16)
{
	arg_p addr16 = arg_dst(vm, _addr16);

	if(0x12 == IR) {
		push(vm, PC, 2);
		CODE_TRACE_COMMENT("(SP = 0x%04X) 0x%08X", SP, addr16->arg);
	} else {
		CODE_TRACE_COMMENT("0x%08X", addr16->arg);
	}

	JMP(addr16->arg);
}

#define mov(_x0, _x1)				_mov(vm, ARG_T(_x0), ARG_T(_x1))
#define mov16(_x0, _x1)				_mov(vm, ARG_T(_x0), ARG_T(_x1))
static void _mov(vm_p vm, arg_type x0, arg_type x1)
{
	int x16 = 0;
	arg_p x[2] = { arg_dst(vm, x0), arg_src(vm, x1) };


	int dst_atDPTRx = (_arg_t_atDPTRx == x[0]->type);
	int dst_atRi = (_arg_t_atRi == x[0]->type);

	int src_atDPTRx = (_arg_t_atDPTRx == x[1]->type);
	int src_atRi = (_arg_t_atRi == x[1]->type);

	int dst_pi = dst_atDPTRx && vm->dpcon.post_inc;

	if(x[1]->type & _arg_t_x16)
		x16 = 1;

	if(dst_atDPTRx) {
		CODE_TRACE_COMMENT("0x%02X --> [0x%08X]%s",
			x[1]->v, x[0]->arg, dst_pi ? "++" : "");
	} else if(dst_atRi) {
		CODE_TRACE_COMMENT("0x%02X --> [0x%02X]",
			x[1]->v, _IR_Ri_);
	} else if(src_atDPTRx) {
		CODE_TRACE_COMMENT("[0x%08X] --> 0x%02X",
			x[1]->arg, x[1]->v);
	} else if(src_atRi) {
		CODE_TRACE_COMMENT("[0x%02X] --> 0x%02X",
			_IR_Ri_, x[1]->v);
	} else if(x16) {
		CODE_TRACE_COMMENT("0x%04X", x[1]->v & 0xffff);
	} else {
		CODE_TRACE_COMMENT("0x%02X", x[1]->v & 0xff);
	}

	arg_wb(vm, x[0], x[1]->v);

	if(dst_pi)
		DPTR++;

//	arg_wb(vm, x[0], x[1]->v);
}

#define movc(_x0, _x1)				mov(_x0, _x1)
#define movx(_x0, _x1)				mov(_x0, _x1)

#define nop()

#define orl(_x0, _x1) alu_box(_x0, _orl_k, _x1)

#define ppop(_x) _ppop(vm, ARG_T(_x))
static void _ppop(vm_p vm, arg_type xt)
{
	arg_p x = arg_dst(vm, xt);
	RES = pop(vm, 1);

	CODE_TRACE_COMMENT("(SP = 0x%04X), 0x%02X", SP, RES);

	arg_wb(vm, x, RES);
}

#define ppush(_x) _ppush(vm, ARG_T(_x))
static void _ppush(vm_p vm, arg_type xt)
{
	arg_p x = arg_src(vm, xt);

	push(vm, x->v, 1);
	CODE_TRACE_COMMENT("(SP = 0x%04X)", SP);
}

#define ret() _ret(vm)
static void _ret(vm_p vm)
{
	const uint32_t new_pc = pop(vm, 2);

	CODE_TRACE_COMMENT("(SP = 0x%04X)", SP);

	JMP(new_pc);
}

#define setb(_bit) _setb(vm, ARG_T(_bit))
static void _setb(vm_p vm, arg_type _bit)
{
	arg_p bit = arg_dst(vm, _bit);

	arg_wb(vm, bit, 1);
}

#define sjmp(_rel) j_cc(_rel, 1)

#define sub16(_x0, _x1) alu16_box(_x0, _sub16_k, _x1)
#define subb(_x0, _x1) alu_box(_x0, _subb_k, _x1)

#define swap(_x) _swap(vm, ARG_T(_x))
static void _swap(vm_p vm, arg_type _x)
{
	arg_p x = arg_src(vm, _x);

	RES = ((x->v & 0xf0f0f0f0) >> 4) | ((x->v & 0x0f0f0f0f) << 4);

	CODE_TRACE_COMMENT("0x%02X --> 0x%02X", x->v, RES);

	arg_wb(vm, x, RES);
}

#define xch(_x0, _x1) _xch(vm, ARG_T(_x0), ARG_T(_x1))
static void _xch(vm_p vm, arg_type x0, arg_type x1)
{
	arg_p x[2] = { arg_src(vm, x0), arg_src(vm, x1) };

	CODE_TRACE_COMMENT("0x%02X, 0x%02X --> 0x%02X, 0x%02X",
		x[0]->v, x[1]->v, x[1]->v, x[0]->v);

	arg_wb(vm, x[0], x[1]->v);
	arg_wb(vm, x[1], x[0]->v);
}

#define xrl(_x0, _x1) alu_box(_x0, _xrl_k, _x1)

/* **** */

#define INST(_esac, _bytes, _cycles, _action, _ops) \
	static void _esac(vm_p vm) \
	{ \
		code_trace_op(vm, _ops, _bytes); \
		\
		_action; \
		\
		CYCLE += (_cycles ? (_cycles - 1) : 0); \
	}

#define ESAC(_op) inst_##_op
#define ESAC_RANGE(_op, _range) ESAC(_op)
#define MASKED(_op, _mask) ESAC(_op)

INST_ESAC_LIST
INST_ESAC_LIST_MASKED
INST_ESAC_LIST_a5
INST_ESAC_LIST_MASKED_a5

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
#define INST(_esac, _bytes, _cycles, _action, _ops) \
	_esac

#undef MASKED
#define MASKED(_op, _mask) \
	if(_op == (IR & _mask)) { \
		inst_##_op(vm); \
	}else

void vm_reset(vm_p vm)
{
	/* X = undefined */

	for(int i = 0; i < 256; i++)
		_SFR_(i) = 0;

	SP = 7;
	DPXL = 1;
	SFR(P0) = 0xff;
	SFR(P1) = 0xff;
	SFR(P2) = 0xff;
	SFR(P3) = 0xff;
	PC = 0xffff0000;
}

static int vm_step_a5(vm_p vm)
{
	do {
		IR = (IR << 8) | ld_code_ia(vm, &PC, 1);
	}while((IR == 0xa5) | (IR == 0xa500));

	INST_ESAC_LIST_MASKED_a5
	switch(IR) {
		default:
				return(-1);
		break;
		INST_ESAC_LIST_a5
	}

	return(0);
}

void vm_step(vm_p vm)
{
	int err = 0;

	IP = PC;
	IR = ld_code_ia(vm, &PC, 1);

	code_trace_start(vm);

//	TRACE("CYCLE: 0x%016llu, IP = 0x%08X, PC = 0x%08X, IR = 0x%08X", CYCLE, IP, PC, IR);

	IXR->argc = 0;

	CYCLE++;

	INST_ESAC_LIST_MASKED
	switch(IR) {
		case 0xa5:
			err = vm_step_a5(vm);
		break;
		default:
				err = -1;
		break;
		INST_ESAC_LIST
	}

	code_trace_out(vm);

	if(err)
		exit(-1);
}
