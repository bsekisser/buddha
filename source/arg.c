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


#undef ARG_ACTION
#define ARG_ACTION(_esac, _action) \
	case _arg_t_##_esac: \
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
		ARG_ESAC(atA_DPTR, (ACC + DPTR));
		ARG_ESAC(atDPTR, DPTR);
		ARG_ESAC(atRi, IR_Ri);
		ARG_ESAC(bit, ld_ia(vm, &PC, 1));
		ARG_ESAC(dir, ld_ia(vm, &PC, 1));
		ARG_ESAC(imm8, ld_ia(vm, &PC, 1));
		ARG_ESAC(imm16, ld_ia(vm, &PC, 2));
		ARG_ESAC(imm16be, ld_ia_be(vm, &PC, 2));
		ARG_ESAC(rel, (int8_t)ld_ia(vm, &PC, 1));
		ARG_ESAC(Rn, IR_Rn);
/* **** */
		default:
			TRACE("type = 0x%02x", type);
			exit(-1);
		case _arg_t_acc:
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
		ARG_ESAC(addr16, x->arg);
		ARG_ESAC(atA_DPTR, ld(vm, x->arg));
		ARG_ESAC(atDPTR, ld(vm, x->arg));
		ARG_ESAC(atRi, ld(vm, _Rn_(x->arg)));
		ARG_ESAC(bit, ld_bit(vm, x->arg));
		ARG_ESAC(dir, ld(vm, x->arg));
		ARG_ESAC(imm8, x->arg);
		ARG_ESAC(imm16, x->arg);
		ARG_ESAC(imm16be, x->arg);
//		ARG_ESAC(rel, x->arg);
		ARG_ESAC(Rn, _IR_Rn_);
		default:
			TRACE("type = 0x%02x", x->type);
			exit(-1);
		break;
	}
	
	return(x);
}

void arg_wb(vm_p vm, arg_p x, uint32_t v)
{
	if(!x) {
		TRACE();
		exit(-1);
	}

	switch(x->type) {
		ARG_ACTION(acc, ACC = v);
		ARG_ACTION(atDPTR, st(vm, x->arg, v));
		ARG_ACTION(atRi, st(vm, _Rn_(x->arg), v));
		ARG_ACTION(bit, st_bit(vm, x->arg, v));
		ARG_ACTION(dir, st(vm, x->arg, v));
		ARG_ACTION(DPTR, DPTR = (DPTR & ~0xffff) | (v & 0xffff));
		ARG_ACTION(DPX, DPTR = (DPTR & ~0xffff) | (v & 0xffff));
		ARG_ACTION(PSW_CY, BSET_AS(PSW, PSW_BIT_CY, v));
		ARG_ACTION(Rn, _IR_Rn_ = v);
		default:
			TRACE("type = 0x%02x", x->type);
			exit(-1);
		break;
	}
}
