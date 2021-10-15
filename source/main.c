#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/bitfield.h"
#include "../include/bswap.h"

#include "on_err.h"
#include "buddha.h"
#include "vm.h"
#include "trace.h"

static void list_code_app_entries(vm_p vm)
{
	printf("     |   idx  |   len  |   ???? |  load@ |   ???? | offset |   dcrc |   tcrc |\n");

	code_idx_ent_p code_table = (void*)vm->code_app;

	for(int i = 0; i < bswap16(code_table[0].idx); i++)
	{
		code_idx_ent_p code_entry = &code_table[i];

		printf("0x%02x | 0x%04x | 0x%04x | 0x%04x | 0x%04x | 0x%04x | 0x%04x | 0x%04x | 0x%04x |\n",
			i, bswap16(code_entry->idx), bswap16(code_entry->len),
			bswap16(code_entry->unk1), bswap16(code_entry->load_at),
			bswap16(code_entry->unk2), bswap16(code_entry->offset),
			bswap16(code_entry->dcrc), bswap16(code_entry->tcrc));
	}
}

static uint16_t load_code_table_entry(vm_p vm, int entry)
{
	code_idx_ent_p code_table = (void*)vm->code_app;
	code_idx_ent_p code_entry = &code_table[entry];

	uint16_t len = bswap16(code_entry->len);
	uint16_t load_at = bswap16(code_entry->load_at);
	uint16_t offset = bswap16(code_entry->offset);
	
	TRACE("offset = 0x%04x, load_at = 0x%04x, length = 0x%04x",
		offset, load_at, len);
	
	uint8_t* src = &vm->code_app[offset];
	uint8_t* dst = &vm->xrom[load_at];
	for(int i = len + 1; i; i--)
		*dst++ = *src++;

	return(load_at);
}

static void* load_mmap(vm_p vm, const char* path, const char* mmats)
{
		int fd;
		struct stat sb;

		ON_1ERR(fail_open, fd = open(path, O_RDONLY));
		ON_1ERR(fail_fstat, fstat(fd, &sb));

		uint8_t* mmat;
		ON_ERR(fail_mmap, MAP_FAILED,
			mmat = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0));

		TRACE("%s = 0x%08x, size = 0x%08x",
			mmats, (int)mmat, (int)sb.st_size);

		close(fd);	fd = -1;

		return(mmat);

fail_mmap:
fail_fstat:
	if(-1 != fd)
		close(fd);
fail_open:
	return((void*)-1);
}

int main(void)
{
	vm_p vm;
	
	ON_0ERR(fail_vm_alloc, vm = calloc(1, sizeof(vm_t)));

	vm->buddha_rom = load_mmap(vm, "archive/buddha_rom.bin", "buddha_rom");
	vm->code_app = load_mmap(vm, "archive/code.app", "code_app");
//	vm->flash_bin = load_mmap(vm, "archive/flash.bin", "flash_bin");

	list_code_app_entries(vm);

	vm_reset(vm);

	memcpy(vm->irom, &vm->buddha_rom[0x8000], 0x1fff);
	memcpy(vm->xram, vm->buddha_rom, 65536);
	memcpy(vm->xrom, vm->buddha_rom, 65536);

	if(1) {
		load_code_table_entry(vm, 0);

		load_code_table_entry(vm, 1);
//		load_code_table_entry(vm, 2);
//		load_code_table_entry(vm, 3);

		JMP(0x2200);
	} else
		JMP(0x8000);

	for(int i = 0; i < 1024; i++)
		vm_step(vm);

	TRACE("CYCLE = 0x%016llx", CYCLE);

	if(vm)
		free(vm);

	return(0);

fail_vm_alloc:
	return(-1);
}
