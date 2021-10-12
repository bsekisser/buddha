#define SETUP_IR_ACC(_x) \
	({ \
		_x->type = _ir_t_implied_acc; \
		_x->v = ACC; \
	})

#define SETUP_IR_ADDR11(_x) \
	({ \
		_x->type = _ir_t_addr11; \
		_x->ea = (PC & ~_BM(11)) | ld_ia(vm, &PC, 1) | ((IR >> 5) << 8); \
	})

#define SETUP_IR_ADDR16(_x) \
	({ \
		_x->type = _ir_t_addr16; \
		_x->ea = ld_ia_be(vm, &PC, 2); \
	});

#define SETUP_IR_BIT(_x, _read) \
	({ \
		_x->type = _ir_t_bit; \
		_x->bit.arg = ld_ia(vm, &PC, 1); \
		_x->bit.pos = _x->bit.arg & 7; \
		_x->bit.ea = _x->bit.arg & 0xf8; \
		_x->bit.bit = 1 << _x->bit.pos; \
		_x->bit.mask = ~_x->bit.bit; \
		\
		if(_read) \
			_x->v = direct_read(vm, _x->bit.ea); \
	})

#define SETUP_IR_DIR(_x, _read) \
	({ \
		_x->type = _ir_t_direct; \
		_x->ea = ld_ia(vm, &PC, 1); \
		\
		if(_read) \
			_x->v = direct_read(vm, _x->ea); \
	})

#define SETUP_IR_IMM8(_x) \
	({ \
		_x->type = _ir_t_imm8; \
		_x->v = ld_ia(vm, &PC, 1); \
	})


#define SETUP_IR_IMPLIED8(_x, _v) \
	({ \
		_x->type = _ir_t_implied_imm8; \
		_x->v = _v; \
	})

#define SETUP_IR_INDIRECT(_x, _read) \
	({ \
		_x->type = _ir_t_indirect; \
		_x->ea = IR_Ri; \
		\
		if(_read) \
			_x->v = indirect_read(vm, _x->ea); \
	})

#define SETUP_IR_Rn(_x) \
	({ \
		_x->type = _ir_t_Rn; \
		_x->v = _Rn_(vm, IR_Rn); \
	})

#define SETUP_IR_REL(_x) \
	({ \
		_x->type = _ir_t_rel; \
		_x->ea = ld_ia(vm, &PC, 1); \
	})

/* **** */

#define SETUP_IR_ACC_DIR(_x1, _x2, _x2read) \
	({ \
		SETUP_IR_ACC(_x1); \
		SETUP_IR_DIR(_x2, _x2read); \
	})


#define SETUP_IR_ACC_IMM8(_x1, _x2) \
	({ \
		SETUP_IR_ACC(_x1); \
		SETUP_IR_IMM8(_x2); \
	})

#define SETUP_IR_ACC_INDIRECT(_x1, _x2, _x2read) \
	({ \
		SETUP_IR_ACC(_x1); \
		SETUP_IR_INDIRECT(_x2, _x2read); \
	})

#define SETUP_IR_ACC_Rn(_x1, _x2) \
	({ \
		SETUP_IR_ACC(_x1); \
		SETUP_IR_Rn(_x2); \
	})

#define SETUP_IR_ACC_V(_x1, _x2, _v) \
	({ \
		SETUP_IR_ACC(_x1); \
		SETUP_IR_IMPLIED8(_x2, _v); \
	})

#define SETUP_IR_BIT_REL(_x1, _x1read, _x2) \
	({ \
		SETUP_IR_BIT(_x1, _x1read); \
		SETUP_IR_REL(_x2); \
	})

#define SETUP_IR_DIR_ACC(_x1, _x1read, _x2) \
	({ \
		SETUP_IR_DIR(_x1, _x1read); \
		SETUP_IR_ACC(_x2); \
	})

#define SETUP_IR_DIR_IMM8(_x1, _x1read, _x2) \
	({ \
		SETUP_IR_DIR(_x1, _x1read); \
		SETUP_IR_IMM8(_x2); \
	})

#define SETUP_IR_DIR_Rn(_x1, _x1read, _x2) \
	({ \
		SETUP_IR_DIR(_x1, _x1read); \
		SETUP_IR_Rn(_x2); \
	})

#define SETUP_IR_Rn_V(_x1, _x2, _v) \
	({ \
		SETUP_IR_Rn(_x1); \
		SETUP_IR_IMPLIED8(_x2, _v); \
	})

#define SETUP_IR_Rn_REL(_x1, _x2) \
	({ \
		SETUP_IR_Rn(_x1); \
		SETUP_IR_REL(_x2); \
	})

#define SETUP_IR_WRjDRk(_x0, _x1) \
	({ \
		uint8_t rJK = ld_ia(vm, &PC, 1); \
		ARG(0)->type = _ir_t_WRj; \
		ARG(0)->r = rJK >> 4 & 0xf; \
		ARG(1)->type = _ir_t_DRk; \
		ARG(1)->r = rJK >> 0xf; \
	})

//#define SETUP_IR_WRk_atDRk_Disp16(

/* **** */

enum {
	_ir_t_nop,
	/* **** */
	_ir_t_addr11,
	_ir_t_addr16,
	_ir_t_bit,
	_ir_t_direct,
	_ir_t_imm8,
	_ir_t_implied_acc,
	_ir_t_implied_imm8,
	_ir_t_indirect,
	_ir_t_rel,
	_ir_t_Ri,
	_ir_t_Rn,
};

static arg_p get_ir_arg(vm_p vm, arg_p arg, int type)
{
	switch(type)
	{
		case	_ir_t_Ri:
			arg->ea = IR & 1;
//			arg->v = get_Ri(vm, ea);
			break;
		case	_ir_t_Rn:
			arg->ea = IR & 7;
//			arg->v = get_Rn(vm, ea);
			break;
		default:
			arg->type = 0;
			return(0);
	}

	arg->type = type;
	return(arg);
}

static void writeback_arg(vm_p vm, arg_p arg)
{
	switch(arg->type)
	{
		
	}
}
