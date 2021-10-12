#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#include "vm.h"

#include "arg.h"
#include "ldst.h"
#include "trace.h"

const char spaces[61] = "                                                             ";

#define TRACE_ESAC(_esac, _f, _args...) \
	case _arg_t_##_esac: \
		SOUT(_f, ##_args); \
	break;

void code_trace_end(vm_p vm)
{
	IXR->trace.pc = PC;
}

void code_trace_op(vm_p vm, const char* op_string)
{
	IXR->trace.op_string = op_string;
}

#define SOUT(_f, _args...) \
	dst += snprintf(dst, end - dst, _f, ##_args);

#undef PC
#define PC IXR->trace.pc

void code_trace_out(vm_p vm)
{
	char out[256], *dst = out, *end = &out[255];

	SOUT("0x%08X: ", IP);
	
	uint8_t count = PC - IP;
	
	for(int i = 0; i < 7; i++)
	{
		if(i < count) {
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
			TRACE_ESAC(addr16, "0x%08lX", _JMP(ARG(i)->arg));
			TRACE_ESAC(atA_DPTR, "@A + DPTR");
			TRACE_ESAC(bit, "$%02X.%01u", BIT_EA(ARG(i)->arg), BIT_POS(ARG(i)->arg));
			TRACE_ESAC(dir, "$%02X", ARG(i)->arg);
			TRACE_ESAC(imm8, "#%02X", ARG(i)->v);
			TRACE_ESAC(atRi, "@R%01u", ARG(i)->arg);
			TRACE_ESAC(rel, "0x%08X", _RJMP(ARG(i)->arg));
			TRACE_ESAC(Rn, "R%01u", ARG(i)->arg);
		}
	}
	
	printf("%s%s%s\n", out, &spaces[dst - out], IXR->trace.comment);
}

void code_trace_start(vm_p vm)
{
	IXR->trace.comment[0] = 0;
	IXR->trace.op_string = "";
}
