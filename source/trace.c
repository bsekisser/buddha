#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "vm.h"

#include "arg.h"
#include "ldst.h"
#include "trace.h"

const char spaces[61] = "                                                             ";

#define TRACE_ESAC(_esac, _f, _args...) \
	case _arg_t_##_esac: \
		SOUT(_f, ##_args); \
	break;

#define SOUT(_f, _args...) \
	dst += snprintf(dst, end - dst, _f, ##_args);

void code_trace_op(vm_p vm, const char* op_string, const uint32_t bytes)
{
	IXR->trace.op_string = op_string;
	IXR->trace.op_bytes = bytes;
}

void code_trace_start(vm_p vm)
{
	IXR->trace.comment[0] = 0;
	IXR->trace.op_string = "";

/* **** !!!!! WARNING !!!! ****
 * 
 * Make SURE this is before modified pc below.
 * 
 * Failing to adhere to this warning
 * 		WILL result in severe confusion!!!!
 * 
 */

	IXR->trace.pc = PC;
}

/* **** */

/* !!!!! WARNING MODIFIED PC BELOW THIS POINT !!!!! */
#undef PC
#define PC							IXR->trace.pc

#define tmp_DRl(_x)					({ tmp = _x; tmp++; })

void code_trace_out(vm_p vm)
{
	char out[256], *dst = out, *end = &out[255];

//	TRACE("CYCLE: 0x%016llu, IP = 0x%08X, PC = 0x%08X, IR = 0x%08X", CYCLE, IP, PC, IR);
	SOUT("0x%08X: ", IP);
	
	uint8_t tmp;
	
	const int32_t pcip_count = PC - IP;

	for(uint32_t i = 0; i < 7; i++)
	{
		if(i < pcip_count) {
			SOUT("%02X ", ld(vm, IP + i));
		} else {
			SOUT("   ");
		}
	}

	SOUT("%5s\t", IXR->trace.op_string);
	
	for(int i = 0; i < IXR->argc; i++)
	{
		if(i)
			SOUT(", ");

		switch(ARG(i)->type) {
			TRACE_ESAC(acc, "A")
			TRACE_ESAC(addr11, "0x%08lX", _JMP(ARG(i)->v));
			TRACE_ESAC(addr16, "0x%08lX", _JMP(ARG(i)->arg));
			TRACE_ESAC(atA_DPTRc, "@A + DPTR");
			TRACE_ESAC(atDPTRx, "@DPTR");
			TRACE_ESAC(bPSW_CY, "C");
			TRACE_ESAC(bit, "$%02X.%01u", BIT_EA(ARG(i)->arg), BIT_POS(ARG(i)->arg));
			TRACE_ESAC(dir, "$%02X", ARG(i)->arg);
			TRACE_ESAC(rDPTR, "DPTR");
			TRACE_ESAC(imm8, "#%02X", ARG(i)->v);
			TRACE_ESAC(imm16, "#%04X", ARG(i)->v);
			TRACE_ESAC(imm16be, "#%04X", ARG(i)->arg);
			TRACE_ESAC(atRi, "@R%01u", IR_Ri);
			TRACE_ESAC(rel, "0x%08X", _RJMP(ARG(i)->arg));
			TRACE_ESAC(Rn, "R%01u", IR_Rn);
/* **** -- a5 -- 16 bit operations */
			TRACE_ESAC(x2, "R%01u:R%01u", tmp_DRl(ARG(i)->arg), tmp);
			TRACE_ESAC(X2y2, "R%01u:R%01u", tmp_DRl(ARG(i)->arg), tmp);
			TRACE_ESAC(x2Y2, "R%01u:R%01u", tmp_DRl(ARG(i)->arg), tmp);
		}
	}
	
	printf("%s%s%s\n", out, &spaces[dst - out], IXR->trace.comment);

	const uint32_t bytes = IXR->trace.op_bytes;

	if((bytes ? bytes : 1) != (pcip_count ? pcip_count : 1))
		TRACE("0x%08X: ***>>> !!! byte count mismatch -- pcip = 0x%08u, expected = 0x%08u!!! <<<***",
			IP, pcip_count, bytes);

	if(!pcip_count) {
		TRACE("?? no pcip_count ??");
		exit(-1);
	}
}
