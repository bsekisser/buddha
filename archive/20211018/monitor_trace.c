#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "min_max.h"

#include "vm.h"
#include "ldst.h"
#include "monitor.h"
#include "trace.h"

#define IP m->trace->ip
#define PC m->trace->pc

#define CS(_x) (_x | ~0xffff)

static void trace_start(monitor_p m)
{
	m->trace.comment[0] = 0;
	m->trace.op_bytes = 0;
	m->trace.op_string = "";
}

void monitor_trace_step(monitor_p m, uint32_t* pat_set, uint32_t pat)
{
	IP = (PC = pat);
	IR = ld_ia(vm, &PC, 1);

	trace_start(vm);

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
