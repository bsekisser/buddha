#define Kb(_x)				((_x) * 1024)
#define Mb(_x)				Kb(Kb(_x))

#define KHz(_x)				((_x) * 1000)
#define MHz(_x)				KHz(KHz(_x))

#define TRACE_ALLOC			1024
#define TRACE_COUNT			(TRACE_DST - TRACE_START)
#define TRACE_DST			vm->trace.dst
#define TRACE_END			&vm->trace.out[TRACE_ALLOC]
#define TRACE_LEFT			(TRACE_END - TRACE_DST)
#define TRACE_START			&vm->trace.out[0]

enum {
	_SFR_P0 = 0x80,
	_SFR_SP = 0x81,
	_SFR_DPL = 0x82,
	_SFR_DPH = 0x83,
	_SFR_PCON = 0x87,
	_SFR_P1 = 0x90,
	_SFR_P2 = 0xa0,
	_SFR_IE = 0xa8,
	_SFR_P3 = 0xb0,
	_SFR_SPH = 0xbe,
	_SFR_PSW = 0xd0,
	_SFR_ACC = 0xe0,
	_SFR_B = 0xf0,
};

#define R(_x)		vm->ram[_r##_x]

#define SFR(_x)		_SFR_(_SFR_##_x)

#define ACC			SFR(ACC)
#define PSW			SFR(PSW)

typedef struct arg_t* arg_p;
typedef struct arg_t {
	uint8_t			type;
	union {
		uint32_t	ea;

		struct {
			uint8_t arg;
			uint8_t bit;
			uint8_t ea;
			uint8_t pos;
			uint8_t mask;
					}bit;
		
		uint8_t		rel;
	};
	
	uint8_t			r;
	uint8_t			v;
}arg_t;

typedef struct vm_t** vm_h;
typedef struct vm_t* vm_p;
typedef struct vm_t {
	uint64_t				cycle;
#define CYCLE				vm->cycle

	struct {
		arg_t				arg[2];
#define ARG(_x)				(&IXR->arg[_x])

		uint32_t			ip;
#define IP					IXR->ip

		uint16_t			ir;
#define IR					IXR->ir
	}ixr;
#define IXR					(&vm->ixr)

	uint32_t				pc;
#define PC					vm->pc

	uint16_t				dptr;
#define DPTR				vm->dptr

	uint8_t					dpxl;
#define DPXL				vm->dpxl
#define DPX					(DPTR | (DPXL << 16))

	uint16_t				sp;
#define SP					vm->sp

	uint8_t					iram[256];		/* 128 - 256 by default */
	uint8_t					irom[Kb(8)];	/* ???? */
	uint8_t					xram[Kb(64)];
	uint8_t					xrom[Kb(64)];	/* buddha rom */

	uint8_t					sfr[128];
#define _SFR_(_x)			vm->sfr[_x & 0x7f]

	uint8_t*				buddha_rom;
	uint8_t*				code_app;
	uint8_t*				flash_bin;

	struct {
		uint8_t				out[TRACE_ALLOC];
		uint8_t*			dst;
	}trace;
}vm_t;

#define IR_Ri				(IR & 1)
#define IR_Rn				(IR & 0x07)

enum {
	PSW_BIT_P,
	PSW_BIT1,
	PSW_BIT_OV,
	PSW_BIT_RS0,
	PSW_BIT_RS1,
	PSW_BIT_F0,
	PSW_BIT_AC,
	PSW_BIT_CY,
};

#define PSW_CY				BTST(PSW, PSW_BIT_CY)
