typedef struct trace_t** trace_h;
typedef struct trace_t* trace_p;

typedef struct monitor_t** monitor_h;
typedef struct monitor_t* monitor_p;

monitor_p monitor_init(vm_p p2vm);
int monitor_main(monitor_p m, int argc, char* argv[]);
uint32_t monitor_trace_step(vm_p vm, trace_h m, uint32_t pat);
