#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include "ldst.h"
#include "sfr.h"
#include "trace.h"

static uint8_t ld_sfr(vm_p vm, uint32_t pat);
static void st_sfr(vm_p vm, uint32_t pat, uint8_t data);

#define SFR_ESAC(_esac, _action) \
	case _SFR_##_esac: \
		_action; \
	break;

uint8_t ld(vm_p vm, uint32_t pat)
{
	uint32_t segment = pat >> 16;
	
	switch(segment) {
		case 0xff ... 0xffff:
			return(ld_code(vm, pat));
		break;
		case 0x01:
			return(ld_xternal(vm, pat));
		break;
		case 0x00:
			return(ld_indirect(vm, pat));
		break;
	}
	return(pat & 0xff);
}

uint8_t ld_bit(vm_p vm, uint32_t pat)
{
	const uint8_t ea = BIT_EA(pat);
	const uint8_t pos = BIT_POS(pat);
	
//	printf("pat = 0x%08x, ea = 0x%02x.%01u\n", pat, ea, pos);
	
	return((ld_direct(vm, ea) >> pos) & 1);
}

uint8_t ld_code(vm_p vm, uint32_t pat)
{
	if((pat & 0xffff) < 0x2000)
		return(vm->irom[pat & 0x1fff]);
	else
		return(vm->xrom[pat & 0xffff]);
}

uint32_t ld_code_ia(vm_p vm, uint32_t* ppat, int count)
{
	uint32_t data = 0;
	
	for(int i = count; i; i--)
		data = (data << 8) | ld_code(vm, (*ppat)++);
	
	return(data);
}

uint8_t ld_direct(vm_p vm, uint32_t pat)
{
	if(pat < 0x80)
		return(vm->iram[pat & 0x7f]);
	else
		return(ld_sfr(vm, pat));
}

uint32_t ld_ia(vm_p vm, uint32_t* ppat, int count)
{
	uint32_t data = 0;
	
	for(int i = count; i; i--)
		data = (data << 8) | ld(vm, (*ppat)++);
	
	return(data);
}

uint8_t ld_indirect(vm_p vm, uint32_t pat)
{
	if(pat < 0x100)
		return(vm->iram[pat & 0xff]);
	else
		return(vm->xram[pat & 0xffff]);
}

static uint8_t ld_sfr(vm_p vm, uint32_t xpat)
{
	const uint8_t pat = xpat & 0x7f;

	switch(xpat & 0xff)
	{
/* 0x81 */	SFR_ESAC(SP, return(SP & 0xff));
/* 0x82 */	SFR_ESAC(DPL, return(DPTR & 0xff));
/* 0x83 */	SFR_ESAC(DPH, return((DPTR >> 8) & 0xff));
/* 0xbe */	SFR_ESAC(SPH, return(SP >> 8));
/* **** */
			case 0x84:			/* ? dp1l ? */
			case 0x85:			/* ? dp1h ? */
/* 0x86 */	case _SFR_DPCON:
//				TRACE("dpcon = 0x%02X", SFR(DPCON));
/* 0x87 */	case _SFR_PCON:
/* 0xa8 */	case _SFR_IE:		/* interrupt control */
			case 0xbc:			/* ? C1CAP2H ? */
			case 0xbd:			/* ? C1CAP3H ? */
			case 0xc4:			/* ? AD4 ? */
/* 0xd0 */	case _SFR_PSW:
/* 0xe0 */	case _SFR_ACC:
/* 0xf0 */	case _SFR_B:
				return(_SFR_(pat));
			break;
			default:
				TRACE("@0x%08X: unhandled sfr (0x%02X, 0x%08X)", IP, pat, xpat);
//				exit(-1);
			break;
	}
	
	return(0xff);
}

uint8_t ld_xternal(vm_p vm, uint32_t pat)
{
	return(vm->xram[pat & 0xffff]);
}

/* **** */

void st_bit(vm_p vm, uint32_t pat, uint8_t set)
{
	const uint8_t ea = BIT_EA(pat);
	const uint8_t pos = BIT_POS(pat);
	const uint8_t bit = 1 << pos;
	const uint8_t mask = ~bit;
	
//	printf("pat = 0x%08x, ea = 0x%02x.%01u\n", pat, ea, pos);

	uint8_t data = ld_direct(vm, ea) & mask;
	data |= (!!set) << pos;
	
	st_direct(vm, ea, data);
}

void st_direct(vm_p vm, uint32_t pat, uint8_t data)
{
	if(pat < 0x80)
		vm->iram[pat & 0x7f] = data;
	else
		st_sfr(vm, pat, data);
}


static void st_dpcon(vm_p vm, uint8_t data)
{
	if(data && (data != 0x10))
		TRACE("data = 0x%02X", data);
	
//	/* 0	-- */ vm->dpcon.dpsel = BEXT(data, 1);
	/* 1	-- reserved */
//	/* 2:3	-- */ vm->dpcon.dp0m = mlBFEXT(data, 2, 3);
//	/* 4:5	-- */ vm->dpcon.dp1m = mlBFEXT(data, 4, 5);
//	/* 6	-- */ vm->dpcon.dpt = BEXT(data, 6);
	/* 7	-- reserved */

	vm->dpcon.post_inc = (1 == ((data >> 4) & 3)); /* shadow dptr inc ? */
	
	
	SFR(DPCON) = data;
}

void st_indirect(vm_p vm, uint32_t pat, uint8_t data)
{
	if(pat < 0x100)
		vm->iram[pat & 0xff] = data;
	else
		vm->xram[pat & 0xffff] = data;
}
	
static void st_sfr(vm_p vm, uint32_t xpat, uint8_t data)
{
	const uint8_t pat = xpat & 0x7f;

	switch(xpat & 0xff)
	{
/* 0x81 */	SFR_ESAC(SP, SP = (SP & ~0xff) | data);
/* 0x82 */	SFR_ESAC(DPL, DPTR = (DPTR & ~0xff) | data);
/* 0x83 */	SFR_ESAC(DPH, DPTR = (DPTR & ~0xff00) | (data << 8));
/* 0x86 */	SFR_ESAC(DPCON, st_dpcon(vm, data));
/* 0xb3 */	SFR_ESAC(SPH, SP = (SP & ~0xff00) | (data << 8));
/* **** */
			case 0x84:			/* ? dp1l ? */
			case 0x85:			/* ? dp1h ? */
/* 0x87 */	case _SFR_PCON:
/* 0xa8 */	case _SFR_IE:		/* interrupt control */
			case 0xbc:			/* ? C1CAP2H ? */
			case 0xbd:			/* ? C1CAP3H ? */
			case 0xc4:			/* ? AD4 ? */
/* 0xd0 */	case _SFR_PSW:
/* 0xe0 */	case _SFR_ACC:
/* 0xf0 */	case _SFR_B:
				_SFR_(pat) = data;
			break;
			default:
				TRACE("@0x%08X: unhandled sfr (0x%02X, 0x%08X)", IP, pat, xpat);
//				exit(-1);
			break;
	}
}

void st_xternal(vm_p vm, uint32_t pat, uint8_t data)
{
	vm->xram[pat & 0xffff] = data;
}
