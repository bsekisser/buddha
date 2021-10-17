#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitfield.h"

#include "vm.h"

#include "arg.h"
#include "ldst.h"
#include "psw.h"
#include "sfr.h"
#include "trace.h"

#define ESAC_ARG(_esac) \
	case _arg_t_##_esac:

#undef ARG_ACTION
#define ARG_ACTION(_esac, _action) \
	ESAC_ARG(_esac) \
		_action; \
	break;

#undef ARG_ESAC
#define ARG_ESAC(_esac, _arg) \
	ARG_ACTION(_esac, x->arg = _arg)

static arg_p arg_arg(vm_p vm, arg_type type)
{
	if(!type)
		return(0);

	arg_p x = ARG(IXR->argc++);
	x->type = type;
	
	switch(type) {
		ARG_ESAC(addr11, ld_code_ia(vm, &PC, 1));
		ARG_ESAC(addr16, ld_code_ia(vm, &PC, 2));
		ARG_ESAC(atA_DPTRc, (ACC + DPTRc));
		ARG_ESAC(atA_PC, (ACC + PC));
		ARG_ESAC(atDPTRx, DPTRx);
		ARG_ESAC(bit, ld_code_ia(vm, &PC, 1));
		ARG_ESAC(cbit, ld_code_ia(vm, &PC, 1));
		ARG_ESAC(dir, ld_code_ia(vm, &PC, 1));
		ARG_ESAC(imm8, ld_code_ia(vm, &PC, 1));
		ARG_ESAC(imm16, ld_code_ia(vm, &PC, 2));
		ARG_ESAC(imm16be, ld_code_ia(vm, &PC, 2));
		ARG_ESAC(rel, (int8_t)ld_code_ia(vm, &PC, 1));
/* **** -- a5 -- 16 bit operations */
		ARG_ESAC(WRx, IR_WRx);
		ARG_ESAC(WRx_WR, IR_WRx_WR);
		ARG_ESAC(WR_WRy, IR_WR_WRy);
		ARG_ESAC(WRx_iWR, IR_WRx_iWR);
		ARG_ESAC(WR_iWRy, IR_WR_iWRy);
		ARG_ESAC(iWRx_WR, IR_iWRx_WR);
		ARG_ESAC(iWR_WRy, IR_iWR_WRy);
/* **** -- ignore -- nop */
		ESAC_ARG(rDPTR)
		ESAC_ARG(acc)
		ESAC_ARG(atRi)
		ESAC_ARG(bPSW_CY);
		ESAC_ARG(Rn)
		break;
/* **** */
		default:
			TRACE("[0x%08X](0x%08X): type = 0x%08X", IP, IR, x->type);
			exit(-1);
		break;
	}

	IXR->trace.pc = PC;

	return(x);
}

#undef ARG_ESAC
#define ARG_ESAC(_esac, _arg) \
	ARG_ACTION(_esac, x->v = _arg)

arg_p arg_dst(vm_p vm, arg_type xt)
{
	arg_p x = arg_arg(vm, xt);
	
	switch(x->type) {
		ARG_ESAC(addr11, ((PC & ~_BM(11)) | ((IR >> 5) << 8) | x->arg));
	}

	return(x);
}

arg_p arg_src(vm_p vm, arg_type xt)
{
	if(!xt)
		return(0);

	arg_p x = arg_arg(vm, xt);
	
	switch(x->type) {
		ARG_ESAC(acc, ACC);
		ARG_ESAC(bbb, BBB);
		ARG_ESAC(atA_DPTRc, ld_code(vm, x->arg));
		ARG_ESAC(atA_PC, ld_code(vm, x->arg));
		ARG_ESAC(atDPTRx, ld_xternal(vm, x->arg));
		ARG_ESAC(atRi, ld_indirect(vm, _IR_Ri_));
		ARG_ESAC(bPSW_CY, PSW_CY);
		ARG_ESAC(bit, ld_bit(vm, x->arg));
		ARG_ESAC(cbit, ~ld_bit(vm, x->arg));
		ARG_ESAC(dir, ld_direct(vm, x->arg));
		ARG_ESAC(rDPTR, DPTR);
		ARG_ESAC(Rn, _IR_Rn_);
/* **** -- a5 -- 16 bit operations */
		ARG_ESAC(WRx, _WRn_(x->arg));
		ARG_ESAC(WRx_WR, _WRn_(x->arg));
		ARG_ESAC(WR_WRy, _WRn_(x->arg));
		ARG_ESAC(WR_iWRy, ld_indirect(vm, _WRn_(x->arg)));
		ARG_ESAC(iWRx_WR, ld_indirect(vm, _WRn_(x->arg)));
		ARG_ESAC(iWR_WRy, _WRn_(x->arg));
/* **** -- common arg */
		ESAC_ARG(imm8)
		ESAC_ARG(imm16)
		ESAC_ARG(imm16be)
		ARG_ESAC(rel, x->arg)
/* **** -- ignore -- nop */
		ESAC_ARG(addr16)
		break;
/* **** */
		default:
			TRACE("[0x%08X](0x%08X): type = 0x%08X", IP, IR, x->type);
			exit(-1);
		break;
	}
	
	return(x);
}

static void st_wr(vm_p vm, uint8_t r, uint32_t v)
{
	_Rn_(r) = v & 0xff;
	_Rn_(r + 1) = (v >> 8) & 0xff;
}

void arg_wb(vm_p vm, arg_p x, uint32_t v)
{
	if(x) switch(x->type) {
		ARG_ACTION(acc, ACC = v);
		ARG_ACTION(bbb, BBB = v);
		ARG_ACTION(atDPTRx, st_xternal(vm, x->arg, v));
		ARG_ACTION(atRi, st_indirect(vm, _IR_Ri_, v));
		ARG_ACTION(bit, st_bit(vm, x->arg, v));
		ARG_ACTION(dir, st_direct(vm, x->arg, v));
		ARG_ACTION(rDPTR, DPTR = v & 0xffff);
		ARG_ACTION(bPSW_CY, BSET_AS(PSW, PSW_BIT_CY, v));
		ARG_ACTION(Rn, _IR_Rn_ = v);
/* **** -- a5 -- 16 bit operations */
		ARG_ACTION(WRx, st_wr(vm, x->arg, v));
		ARG_ACTION(WRx_WR, st_wr(vm, x->arg, v));
		ARG_ACTION(WRx_iWR, st_wr(vm, x->arg, v));
		ARG_ACTION(iWRx_WR, st_indirect(vm, _WRn_(x->arg), v));
/* **** */
		default:
			TRACE("[0x%08X](0x%08X): type = 0x%08X", IP, IR, x->type);
			exit(-1);
		break;
	} else {
		TRACE("???");
		exit(-1);
	}
}
