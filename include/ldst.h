uint8_t ld(vm_p vm, uint32_t pat);
uint8_t ld_code(vm_p vm, uint32_t pat);
uint32_t ld_code_ia(vm_p vm, uint32_t* pat, int count);
uint8_t ld_bit(vm_p vm, uint32_t pat);
uint8_t ld_direct(vm_p vm, uint32_t pat);
uint32_t ld_ia(vm_p vm, uint32_t* pat, int count);
uint8_t ld_indirect(vm_p vm, uint32_t pat);
uint8_t ld_xternal(vm_p vm, uint32_t pat);

void st_bit(vm_p vm, uint32_t pat, uint8_t data);
void st_direct(vm_p vm, uint32_t pat, uint8_t data);
void st_indirect(vm_p vm, uint32_t pat, uint8_t data);
void st_xternal(vm_p vm, uint32_t pat, uint8_t data);
