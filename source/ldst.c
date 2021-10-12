#include <stdint.h>
#include <stdio.h>

#include "vm.h"
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
	uint8_t segment = (pat >> 16) & 0xff;
	uint8_t page = (pat >> 8) & 0xff;
	
	switch(segment) {
		case 0xff:
			if(page <= 0x1f)
				return(vm->irom[pat & 0x1fff]);
			else
				return(vm->xrom[pat & 0xffff]);
		break;
		case 0x01:
			return(vm->xram[pat & 0xffff]);
		break;
		case 0x00:
			if(pat < 0x80)
				return(vm->iram[pat & 0xff]);
			else
				return(ld_sfr(vm, pat));
		break;
	}

	return(0xff);
//	return((pat >> 8) & 0xff);
}

uint8_t ld_bit(vm_p vm, uint8_t pat)
{
	const uint8_t ea = pat & ~0x7f;
	const uint8_t pos = pat & 7;
	
	return((ld(vm, ea) >> pos) & 1);
}

uint32_t ld_ia(vm_p vm, uint32_t *ppat, int count)
{
	uint32_t data = 0;
	
	for(int i = 0; i < count; i++)
		data |= (ld(vm, (*ppat)++) << (i << 3));

	return(data);
}
	

uint32_t ld_ia_be(vm_p vm, uint32_t* ppat, int count)
{
	uint32_t data = 0;
	
	for(int i = count; i; i--)
		data = data << 8 | ld(vm, (*ppat)++);

	return(data);
}

static uint8_t ld_sfr(vm_p vm, uint32_t pat)
{
	switch(pat & 0xff)
	{
/* 0x81 */	SFR_ESAC(SP, return(SP & 0xff));
/* 0x82 */	SFR_ESAC(DPL, return(DPTR & 0xff));
/* 0x83 */	SFR_ESAC(DPH, return((DPTR >> 8) & 0xff));
/* 0xbe */	SFR_ESAC(SPH, return(SP >> 8));
/* **** */
			case 0x86:
/* 0xa8 */	case _SFR_IE:		/* interrupt control */
			case 0xbc:			/* ? C1CAP2H ? */
			case 0xbd:			/* ? C1CAP3H ? */
/* 0xd0 */	case _SFR_PSW:
/* 0xe0 */	case _SFR_ACC:
/* 0xf0 */	case _SFR_B:
				return(_SFR_(pat));
			break;
			default:
				TRACE("unhandled sfr (0x%02X)", pat & 0xff);
//				exit(-1);
			break;
	}
	
	return(0xff);
}

/* **** */

void st(vm_p vm, uint32_t pat, uint8_t data)
{
	uint8_t segment = (pat >> 16) & 0xff;
//	uint8_t page = (pat >> 8) & 0xff;

	switch(segment) {
		case 0x01:
			vm->xram[pat & 0xffff] = data;
		break;
		case 0x00:
			if(pat < 80)
				vm->iram[pat & 0xff] = data;
			else
				st_sfr(vm, pat, data);
		break;
	}
}

void st_bit(vm_p vm, uint8_t pat, uint8_t set)
{
	const uint8_t ea = pat & ~0x7f;
	const uint8_t pos = pat & 7;
	const uint8_t bit = 1 << pos;
	const uint8_t mask = ~bit;
	
	uint8_t data = ld(vm, ea) & mask;
	data |= bit;
	
	st(vm, ea, data);
}

static void st_sfr(vm_p vm, uint32_t pat, uint8_t data)
{
	switch(pat & 0xff)
	{
/* 0x81 */	SFR_ESAC(SP, SP = (SP & ~0xff) | data);
/* 0x82 */	SFR_ESAC(DPL, DPTR = (DPTR & ~0xff) | data);
/* 0x83 */	SFR_ESAC(DPH, DPTR = (DPTR & ~0xff00) | (data << 8));
/* 0xb3 */	SFR_ESAC(SPH, SP = (SP & ~0xff00) | (data << 8));
/* **** */
			case 0x86:
/* 0x87 */	case _SFR_PCON:
/* 0xa8 */	case _SFR_IE:		/* interrupt control */
			case 0xbc:			/* ? C1CAP2H ? */
			case 0xbd:			/* ? C1CAP3H ? */
/* 0xd0 */	case _SFR_PSW:
/* 0xe0 */	case _SFR_ACC:
/* 0xf0 */	case _SFR_B:
				_SFR_(pat) = data;
			break;
			default:
				TRACE("unhandled sfr (0x%02X)", pat & 0xff);
		//						exit(-1);
			break;
	}
}
