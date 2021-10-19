#include <ctype.h>
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
	trace_p							trace;

//	char*							src;
	uint32_t						fill;
	uint32_t						pat;
	uint32_t						to_pat;
	uint32_t						segment;
	uint32_t						stack[0x10];

	uint8_t							sp;

}monitor_t;

/* **** -- utility functions */
/* **** -- command action functions */

static void list(monitor_p m)
{
//	TRACE();

	m->pat = vm_step_trace(m->vm, m->pat, 20, 0);
}

static void list_to(monitor_p m, uint32_t to_pat)
{
	while(m->pat < to_pat) {
		m->pat = vm_step_trace(m->vm, m->pat, 1, 0);
	}
}

void dump(monitor_p m, uint8_t count)
{
//	TRACE("count = 0x%08x", count);

	m->pat |= (m->segment << 16);

	do {
		uint8_t out[9], *dst = out;

		printf("0x%08X: ", m->pat);

		do {
			uint8_t c = ld(m->vm, m->pat++);
			printf("%02X ", c);
			
			c &= 0x7f;
			*dst++ = ((c < ' ') ? ' ' : c);
		}while((--count) && (m->pat & 7));

		*dst = 0;

		printf("- %s\n", out);
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
	uint32_t count = 8 - (m->pat & 7);
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

static int parse_hex(char** p2src, uint32_t* p2v) {
	char *src = *p2src;
	uint32_t v = 0;

	while(*src && isxdigit(*src)) {
		char c = *src++;

		v = ((c & 0xf) + (c > '9' ? 9 : 0)) | (v << 4);

//		TRACE("src = 0x%08X, *src = 0x%02X, c = 0x%02X, v = 0x%08X", src, *src, c, v);

		if(!*src || !isxdigit(*src)) {
			*p2src = src;
			*p2v = v;

			return(1);
		}
	}

	return(0);
}

static int match_char(char** p2src, const char c)
{
	char *src = *p2src;

//	TRACE("src = %08X, *src = %02X", src, *src);

	if(*src && (c == *src++)) {
		*p2src = src;
		return(1);
	}
	
	return(0);
}

enum {
	_nop_k,
/* **** */
	_nop_nop_k = 1 << 8,

	_dump_k,
	_dump_line_k,
	_fill_k,
	_int_k,
	_list_k,
/* **** */
};

enum {
	_flag_nop_k,
	_flag_fill_k,
	_flag_range_k,
	_flag_set_segment_k,
};

#define _flag(_x) (0x80 << _flag_##_x##_k)
#define _set_flag(_x) flags |= _flag(_x)
#define _test_flag(_x) (flags & _flag(_x))

static int32_t parse_token(monitor_p m, char** src, uint32_t* v)
{
	uint8_t c = **src;
	
//	TRACE("src = 0x%08X, *src = 0x%02X, c = 0x%02X", src, *src, c);

	while(c && isblank(c))
		c = *(*src)++;

//	TRACE("src = 0x%08X, *src = 0x%02X, c = 0x%02X", src, *src, c);

	if(parse_hex(src, v))
		return(_int_k);

//	TRACE("src = 0x%08X, *src = 0x%02X, c = 0x%02X", src, *src, c);

	return(c);
}

uint32_t _pop(monitor_p m)
{
	m->sp--;
	m->sp &= 0xf;
	return(m->stack[m->sp]);
}

void _push(monitor_p m, uint32_t v)
{
	m->stack[m->sp] = v;
	m->sp++;
	m->sp &= 0xf;
}


static void parse_line(monitor_p m, char* line)
{
	int action = 0, flags = 0;
	char *src = line;
	uint32_t c, v;
	
	while(*src) {
//		TRACE("src = %08X, *src = %02X", src, *src);

		if(match_char(&src, '.'))
			_set_flag(range);
		else if((c = parse_token(m, &src, &v))) {
			if(_int_k == c) {
				if(match_char(&src, '/'))
					m->segment = v;
				else if(match_char(&src, '<'))
					m->fill = v;
				else {
					action = _dump_k;
					if(_test_flag(range)) {
//		TRACE("src = %08X, *src = %02X", src, *src);
						v += m->pat > v ? m->pat : 0;
						m->to_pat = v;
					} else {
//		TRACE("src = %08X, *src = %02X", src, *src);
						m->pat = v;
					}
				}
			}
		} else if(match_char(&src, 'l') || match_char(&src, 'L')) {
			action = _list_k;
		} else if(match_char(&src, '\n')) {
				action = action ? action : _dump_line_k;
		}
	}
	
	switch(action | flags)
	{
		case _dump_k:
			dump(m, 1);
		break;
		case _dump_k | _flag(range):
			dump_to(m, m->to_pat);
		break;
		case _dump_line_k:
//		case _dump_line_k | _flag(range):
			dump_line(m);
		break;
		case _list_k:
			list(m);
		break;
		case _list_k | _flag(range):
			list_to(m, m->to_pat);
		break;
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
