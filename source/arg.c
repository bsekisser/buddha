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
		ARG_ESAC(addr11, ld_ia(vm, &PC, 1));
		ARG_ESAC(addr16, ld_ia_be(vm, &PC, 2));
		ARG_ESAC(atA_DPTRc, (ACC + DPTRc));
		ARG_ESAC(atDPTRx, DPTRx);
		ARG_ESAC(bit, ld_ia(vm, &PC, 1));
		ARG_ESAC(dir, ld_ia(vm, &PC, 1));
		ARG_ESAC(imm8, ld_ia(vm, &PC, 1));
		ARG_ESAC(imm16, ld_ia(vm, &PC, 2));
		ARG_ESAC(imm16be, ld_ia_be(vm, &PC, 2));
		ARG_ESAC(rel, (int8_t)ld_ia(vm, &PC, 1));
/* **** -- a5 -- 16 bit operations */
		ARG_ESAC(x2, (IR & 3) << 1);
		ARG_ESAC(X2y2, ((IR >> 1) & 6));
		ARG_ESAC(x2Y2, (IR & 3) << 1);
/* **** -- ignore -- nop */
		ESAC_ARG(rDPTR)
		ESAC_ARG(acc)
		ESAC_ARG(atRi)
		ESAC_ARG(bPSW_CY);
		ESAC_ARG(Rn)
		break;
/* **** */
		default:
			TRACE("type = 0x%02x", type);
			exit(-1);
		break;
	}

	return(x);
}

arg_p arg_dst(vm_p vm, arg_type xt)
{
	return(arg_arg(vm, xt));
}

#undef ARG_ESAC
#define ARG_ESAC(_esac, _arg) \
	ARG_ACTION(_esac, x->v = _arg)

arg_p arg_src(vm_p vm, arg_type xt)
{
	if(!xt)
		return(0);

	arg_p x = arg_arg(vm, xt);
	
	switch(x->type) {
		ARG_ESAC(acc, ACC);
		ARG_ESAC(addr11, ((PC & ~_BM(11)) | ((IR >> 5) << 8) | x->arg));
		ARG_ESAC(atA_DPTRc, ld(vm, x->arg));
		ARG_ESAC(atDPTRx, ld(vm, x->arg));
		ARG_ESAC(atRi, ld(vm, _IR_Ri_));
		ARG_ESAC(bPSW_CY, PSW_CY);
		ARG_ESAC(bit, ld_bit(vm, x->arg));
		ARG_ESAC(dir, ld(vm, x->arg));
		ARG_ESAC(rDPTR, DPTR);
		ARG_ESAC(Rn, _IR_Rn_);
/* **** -- a5 -- 16 bit operations */
		ARG_ESAC(x2, _DRn_(x->arg));
		ARG_ESAC(X2y2, _DRn_(x->arg));
		ARG_ESAC(x2Y2, _DRn_(x->arg));
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
			TRACE("type = 0x%02x", x->type);
			exit(-1);
		break;
	}
	
	return(x);
}

static void arg_wb_dr(vm_p vm, uint8_t r, uint32_t v)
{
	_Rn_(r) = v & 0xff;
	_Rn_(r + 1) = (v >> 8) & 0xff;
}

void arg_wb(vm_p vm, arg_p x, uint32_t v)
{
	if(x) switch(x->type) {
		ARG_ACTION(acc, ACC = v);
		ARG_ACTION(atDPTRx, st(vm, x->arg, v));
		ARG_ACTION(atRi, st(vm, _IR_Ri_, v));
		ARG_ACTION(bit, st_bit(vm, x->arg, v));
		ARG_ACTION(dir, st(vm, x->arg, v));
		ARG_ACTION(rDPTR, DPTR = v & 0xffff);
		ARG_ACTION(bPSW_CY, BSET_AS(PSW, PSW_BIT_CY, v));
		ARG_ACTION(Rn, _IR_Rn_ = v);
/* **** -- a5 -- 16 bit operations */
		ARG_ACTION(x2, arg_wb_dr(vm, x->arg, v));
/* **** */
		default:
			TRACE("type = 0x%02x", x->type);
			exit(-1);
		break;
	} else {
		TRACE("???");
		exit(-1);
	}
}
