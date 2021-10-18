#define Kb(_x)				((_x) * 1024)
#define Mb(_x)				Kb(Kb(_x))

#define KHz(_x)				((_x) * 1000)
#define MHz(_x)				KHz(KHz(_x))

#define _JMP(_ea) (~0xffffUL + (_ea & 0xffffUL))
#define JMP(_ea) PC = _JMP(_ea);

#define _RJMP(_ea) (PC + (int8_t)_ea)
#define RJMP(_ea) PC = _RJMP((int8_t)_ea);

typedef struct arg_t** arg_h;
typedef struct arg_t* arg_p;
typedef struct arg_t {
	uint32_t				v;
	uint32_t				arg;
	uint32_t				type;
}arg_t;

typedef struct ixr_t* ixr_p;
typedef struct ixr_t {
	uint32_t				res;
#define RES					IXR->res

	arg_t					arg[2];
	int						argc;
#define ARG(_x)				(&IXR->arg[_x])

	uint32_t				ip;
#define IP					IXR->ip

	uint32_t				ir;
#define IR					IXR->ir

	struct {
		char				comment[256];
		uint32_t			op_bytes;
		const char*			op_string;
		uint32_t			pc;
	}trace;

	struct {
		uint8_t				wb:1;
#define WB					IXR->wb
	};
}ixr_t;

typedef struct vm_t** vm_h;
typedef struct vm_t* vm_p;
typedef struct vm_t {
	uint64_t				cycle;
#define CYCLE				vm->cycle

	ixr_t					ixr;
#define IXR					(&vm->ixr)

	uint32_t				pc;
#define PC					vm->pc

	uint16_t				dptr;
#define DPTR				vm->dptr

	struct {
		uint8_t				post_inc;
	}dpcon;

	uint8_t					dpxl;
#define DPXL				vm->dpxl
#define DPX					((DPTR & 0xffff) | (DPXL << 16))

	uint16_t				sp;
#define SP					vm->sp

	uint8_t					iram[256];		/* 128 - 256 by default */
	uint8_t					irom[Kb(8)];	/* ???? */
	uint8_t					xram[Kb(64)];
	uint8_t					xrom[Kb(64)];	/* buddha rom */

	uint8_t					sfr[128];
#define _SFR_(_x)			vm->sfr[_x & 0x7f]
#define SFR(_x)				_SFR_(_SFR_##_x)

	uint8_t*				buddha_rom;
	uint8_t*				code_app;
	uint8_t*				flash_bin;
}vm_t;

vm_p vm_init(void);
void vm_reset(vm_p vm);
void vm_step(vm_p vm);

#define ACC			SFR(ACC)
#define BBB			SFR(B)
#define PSW			SFR(PSW)

#define _Rn_(_r) \
	vm->iram[(PSW & (3 << 3)) | ((_r) & 7)]

#define _WRn_(_r) \
	(_Rn_(_r) | (_Rn_((_r) + 1) << 8))

#define IR_Ri						(IR & 1)
#define IR_Rn						(IR & 7)

#define IR_WRx						((IR & 3) << 1)
#define IR_WRx_WR					((IR >> 1) & 6)
#define IR_WR_WRy					IR_WRx
#define IR_WRx_iWR					IR_WRx_WR
#define IR_WR_iWRy					IR_WRx
#define IR_iWRx_WR					IR_WRx
#define IR_iWR_WRy					IR_WRx_WR

#define _IR_Ri_						_Rn_(IR_Ri)
#define _IR_Rn_						_Rn_(IR_Rn)

#define	BIT_EA(_x)					((_x) & 0xf8)
#define BIT_POS(_x)					((_x) & 7)

#define DPTRc						(~0xffff | (DPTR & 0xffff))
#define DPTRx						(0x00010000 | (DPTR & 0xffff))
