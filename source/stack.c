#include <stdint.h>

#include "vm.h"

#include "ldst.h"

uint32_t pop(vm_p vm, int count)
{
	uint32_t data = 0;

	for(int i = count; i; i--)
	{
		data = (data << 8);
		data |= vm->iram[SP] & 0xff;
		SP = (SP - 1) & 0xff;
	}

	return(data);
}

void push(vm_p vm, uint32_t data, int count)
{
	for(int i = count; i; i--)
	{
		SP = (SP + 1) & 0xff;
		vm->iram[SP] = data & 0xff;
		data >>= 8;
	}
}
