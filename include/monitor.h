typedef struct monitor_t** monitor_h;
typedef struct monitor_t* monitor_p;

monitor_p monitor_init(vm_p p2vm);
int monitor_main(monitor_p m, int argc, char* argv[]);
