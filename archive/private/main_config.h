	memcpy(vm->irom, &vm->buddha_rom[0x8000], 0x1fff);
//	memcpy(vm->xram, vm->buddha_rom, 65536);
	memcpy(vm->xrom, vm->buddha_rom, 65536);

	if(1) {
		load_code_table_entry(vm, 0);

//		load_code_table_entry(vm, 1);
		load_code_table_entry(vm, 2);
//		load_code_table_entry(vm, 3);

		JMP(0x2200);
	}
