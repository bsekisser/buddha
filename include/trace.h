#define TRACE(_f, _args...) \
	printf("%s:%04u: " _f "\n", __FUNCTION__, __LINE__, ## _args);

#define CODE_TRACE_COMMENT(_f, _args...) \
	snprintf(IXR->trace.comment, 255, "/* " _f " */", ##_args);

void code_trace_op(vm_p vm, const char *op_string, const uint32_t bytes);
void code_trace_out(vm_p vm);
void code_trace_start(vm_p vm);
