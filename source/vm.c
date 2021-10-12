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
	_cmp16_k,
	_dec_k,
	_inc_k,
	_or_k,
	_mov_k,
	_sub_k,
	_xor_k,

	/* **** */

	_anl_k = _and_k,
	_orl_k = _or_k,
	_xrl_l = _xor_k,
};

#define alu_box(_x0, _esac, _x1) _alu_box(vm, _esac, _x0, _x1)
static void _alu_box(vm_p vm, int esac, arg_type x0, arg_type x1)
{
	arg_p x[2] = { arg_src(vm, x0), arg_src(vm, x1) };

	const char* ops;
	int wb = 1;
	RES = x[0]->v;

	uint32_t x1v = x[1] ? x[1]->v : 1;

	switch(esac) {
		case	_adc_k:
			ops = "+";
			RES += (x[1]->v + !!PSW_CY);
		break;
		case	_add_k:
			ops = "+";
			RES += x[1]->v;
		break;
		case	_and_k:
			ops = "&";
			RES &= x[1]->v;
		break;
		case	_dec_k:
			ops = "-";
			RES--;
		break;
		case	_inc_k:
			ops = "+";
			RES++;
		break;
		case	_mov_k:
			ops = "--";
			RES = x[1]->v;
		break;
		case	_or_k:
			ops = "|";
			RES |= x[1]->v;
		break;
		case	_cmp16_k:
			wb = 0;
		case	_sub_k:
			ops = "-";
			RES -= x[1]->v;
		break;
		case	_xor_k:
			ops = "^";
			RES ^= x[1]->v;
		break;
	}

	switch(esac) {
		case _mov_k:
			CODE_TRACE_COMMENT("0x%02X", RES);
		break;
		default:
			CODE_TRACE_COMMENT("0x%02X %s 0x%02X --> 0x%02X", x[0]->v, ops, x1v, RES);
		break;
	}

	code_trace_end(vm);

	if(wb)
		arg_wb(vm, x[0], RES);
}

/* **** */

#define add(_x0, _x1) alu_box(ARG_T(_x0), _add_k, ARG_T(_x1))

#define clr(_x) _clr(vm, ARG_T(_x))
static void _clr(vm_p vm, arg_type _x)
{
	arg_p x = arg_dst(vm, _x);

	code_trace_end(vm);

	arg_wb(vm, x, 0);
}


#define djnz(_x, _rel) _djnz(vm, ARG_T(_x), ARG_T(_rel))
static void _djnz(vm_p vm, arg_type _x, arg_type _rel)
{
	arg_p x = arg_src(vm, _x);
	arg_p rel = arg_dst(vm, _rel);
	RES = x->v - 1;

	CODE_TRACE_COMMENT("(0x%04X - 1 --> 0x%04X) will%sjump",
		x->v, RES, RES ? " " : " not ");

	code_trace_end(vm);

	arg_wb(vm, x, RES);
	if(RES)
		RJMP(rel->arg);
}

#define inc(_x)						alu_box(ARG_T(_x), _inc_k, 0)

#define lcall ljmp
#define ljmp(_addr16) _ljmp(vm, ARG_T(_addr16))
static void _ljmp(vm_p vm, arg_type _addr16)
{
	arg_p addr16 = arg_dst(vm, _addr16);

	if(IR & 0x10) {
		push(vm, PC, 2);
		CODE_TRACE_COMMENT("(SP = 0x%02X) 0x%08X", SP, addr16->arg);
	} else {
		CODE_TRACE_COMMENT("0x%08X", addr16->arg);
	}

	code_trace_end(vm);

	JMP(addr16->arg);
}

#define mov(_x0, _x1)				alu_box(ARG_T(_x0), _mov_k, ARG_T(_x1))

#define nop()

#define ppop(_x) _ppop(vm, ARG_T(_x))
static void _ppop(vm_p vm, arg_type xt)
{
	arg_p x = arg_dst(vm, xt);
	RES = pop(vm, 1);

	CODE_TRACE_COMMENT("(SP = 0x%04X), 0x%02X", SP, RES);

	code_trace_end(vm);

	arg_wb(vm, x, RES);
}

#define ppush(_x) _ppush(vm, ARG_T(_x))
static void _ppush(vm_p vm, arg_type xt)
{
	arg_p x = arg_src(vm, xt);
	
	push(vm, x->v, 1);
	CODE_TRACE_COMMENT("(SP = 0x%04X)", SP);
	
	code_trace_end(vm);
}


#define xch(_x0, _x1) _xch(vm, ARG_T(_x0), ARG_T(_x1))
static void _xch(vm_p vm, arg_type x0, arg_type x1)
{
	arg_p x[2] = { arg_src(vm, x0), arg_src(vm, x1) };
	
	CODE_TRACE_COMMENT("0x%02X, 0x%02X --> 0x%02x, 0x%02X",
		x[0]->v, x[1]->v, x[1]->v, x[0]->v);

	code_trace_end(vm);

	arg_wb(vm, x[0], x[1]->v);
	arg_wb(vm, x[1], x[0]->v);
}

/* **** */

#define INST(_esac, _bytes, _cycles, _action, _ops) \
	static void _esac(vm_p vm) \
	{ \
		code_trace_op(vm, _ops); \
		\
		_action; \
		\
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

static void inst_a5(vm_p vm)
{
//	INST_ESAC_LIST_a5
}

void vm_step(vm_p vm)
{
	int err = 0;
	
	IP = PC;
	IR = ld_ia(vm, &PC, 1);
		
	IXR->argc = 0;
	code_trace_start(vm);

	CYCLE++;

//	TRACE("CYCLE: 0x%016llu, IP = 0x%04X, IR = 0x%08X\n", CYCLE, IP, IR);

	switch(IR) {
		default:
			if(0xa5 == IR)
				inst_a5(vm);
			else 
			INST_ESAC_LIST_MASKED
			{
				code_trace_end(vm);
				err = -1;
			}
		break;
		INST_ESAC_LIST
	}

	code_trace_out(vm);

	if(err)
		exit(-1);
}
