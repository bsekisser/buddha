void _add_set_flags(vm_p vm, uint8_t v1, uint8_t v2, uint8_t res);

uint8_t ld(vm_p vm, uint32_t pat);
uint32_t ld_ia(vm_p vm, uint32_t *ppat, int count);
uint32_t ld_ia_be(vm_p vm, uint32_t* ppat, int count);

uint32_t pop(vm_p vm, int count);
void push(vm_p vm, uint32_t data, int count);

void st(vm_p vm, uint32_t pat, uint8_t data);

void vm_test_step(vm_p vm);
