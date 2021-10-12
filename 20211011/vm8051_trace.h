#define TRACE(_f, _args...) \
	printf("%s:%04u: " _f "\n", __FUNCTION__, __LINE__, ## _args);

#define CODE_TRACE_START() \
	({ \
		*(TRACE_DST = TRACE_START) = 0; \
		TRACE_DST += snprintf(TRACE_DST, TRACE_LEFT, "0x%08X: ", IP); \
		if(IR >> 8) \
			TRACE_DST += snprintf(TRACE_DST, TRACE_LEFT, "%02X ", (IR >> 8) & 0xff); \
		\
		TRACE_DST += snprintf(TRACE_DST, TRACE_LEFT, "%02X ", IR & 0xff); \
		\
	})

#define CODE_TRACE_OP(_count, _fop, _farg, _args...) \
	({ \
		for(int i = 0; i < 6; i++) \
		{ \
			if(i < _count) \
				TRACE_DST += snprintf(TRACE_DST, TRACE_LEFT, "%02X ", ld(vm, (IP + 1) + i)); \
			else \
				TRACE_DST += snprintf(TRACE_DST, TRACE_LEFT, "   "); \
		} \
		TRACE_DST += snprintf(TRACE_DST, TRACE_LEFT, "%5s\t" _farg, _fop, ##_args); \
	})

#define CODE_TRACE_COMMENT(_f, _args...) \
	({ \
		TRACE_DST += snprintf(TRACE_DST, TRACE_LEFT, "%s /* " _f " */", \
			&spaces[TRACE_COUNT], ##_args); \
	})

#define CODE_TRACE_ALU(_op) \
	CODE_TRACE_COMMENT("0x%04x %s 0x%04x --> 0x%04x", \
		ARG(0)->v, #_op, ARG(1)->v, res);

#define CODE_TRACE_COMPLETE() \
	printf("%s\n", TRACE_START);
