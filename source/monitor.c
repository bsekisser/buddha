#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "min_max.h"

#include "vm.h"
#include "ldst.h"
#include "monitor.h"
#include "trace.h"

typedef struct monitor_t {
	vm_p							vm;
	char*							src;
	uint32_t						pat;
	uint32_t						to_pat;
	uint32_t						segment;
}monitor_t;

/* **** -- utility functions */

uint8_t ld(vm_p vm, uint32_t pat)
{
	uint32_t segment = pat >> 16;
	
	switch(segment) {
		case 0xff ... 0xffff:
			return(ld_code(vm, pat));
		break;
		case 0x01:
			return(ld_xternal(vm, pat));
		break;
		case 0x00:
			return(ld_indirect(vm, pat));
		break;
	}
	return(pat & 0xff);
}

/* **** -- command action functions */

void dump(monitor_p m, uint8_t count)
{
//	TRACE("count = 0x%08x", count);

	m->pat |= (m->segment << 16);

	do {
		printf("0x%08X: ", m->pat);

		do {
			printf("%02X ", ld(m->vm, m->pat++));
		}while((--count) && (m->pat & 7));
	
		printf("\n");
	}while(count);
}

void dump_to(monitor_p m, uint32_t to_pat)
{
	uint32_t count = ++to_pat - m->pat;

//	TRACE("to_pat = 0x%08x, count = 0x%08x", to_pat, count);

	if(0 > count)
		TRACE("ERR: to < pat");

	dump(m, count);
}

static void dump_line(monitor_p m)
{
	uint8_t count = 8 - (m->pat & 7);
	dump(m, count);
}

/* **** */

monitor_p monitor_init(vm_p p2vm)
{
	monitor_p m = calloc(1, sizeof(monitor_t));
	if(!m)
		return(0);
	
	m->vm = p2vm;
	
	return(m);
}

static size_t scanf_kludge(monitor_p m, size_t count, int* len)
{
	int result = count && (-1 != count);
//	TRACE("m->src = 0x%08x, count = 0x%08x, len = 0x%08x, result = 0x%08x", m->src, count, *len, result);

	m->src += *len;
	return(result);
}

#define SSCANF(_f, _args...) scanf_kludge(m, sscanf(m->src, _f "%n", ##_args, &len), &len)

//	ESAC(_esac, _sscanf, _action)
#define CMD_LIST \
	ESAC(dump_line, 0, dump_line(m)) \
	ESAC(dump, SSCANF("%8x", &m->pat), dump(m, 1)) \
	ESAC(seg_dump, SSCANF("%2x/%8x", &m->segment, &m->pat), dump(m, 1)) \
	ESAC(dump_to, SSCANF(".%8x", &m->to_pat), dump_to(m, m->to_pat))

#undef ESAC
#define ESAC(_esac, _sscanf, _action) \
	_##_esac##_k,

enum {
	CMD_LIST
};

static void parse_line(monitor_p m, char* line)
{
	m->src = line;
	int action = 0, len = 0;

#undef ESAC
#define ESAC(_esac, _sscanf, _action) \
	if(_sscanf) action = _##_esac##_k;

	CMD_LIST

#undef ESAC
#define ESAC(_esac, _sscanf, _action) \
	case _##_esac##_k: \
		_action; \
	break;

	switch(action) {
		CMD_LIST
	}
}

int monitor_main(monitor_p m, int argc, char* argv[])
{
	FILE *stream = stdin;
	char *line = 0;
	size_t len = 0;
	ssize_t nread;
	
	while((nread = getline(&line, &len, stream)) != -1) {
//		TRACE("line = 0x%08x, len = 0x%08X, nread = 0x%08x\n", (int)line, len, nread);

		parse_line(m, line);
	}
	
	free(line);
	return(0);
}
