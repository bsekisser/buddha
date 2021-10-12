uint8_t ld(vm_p vm, uint32_t pat);
uint8_t ld_bit(vm_p vm, uint32_t pat);
uint32_t ld_ia(vm_p vm, uint32_t *ppat, int count);
uint32_t ld_ia_be(vm_p vm, uint32_t* ppat, int count);

void st(vm_p vm, uint32_t pat, uint8_t data);
void st_bit(vm_p vm, uint32_t pat, uint8_t data);
