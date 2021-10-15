#undef ARG_ACTION
#define ARG_ACTION(_esac, _action) \
	case _arg_t_##_esac: \
		_action; \
	break;

typedef enum arg_type {
	_arg_t_nop,
	_arg_t_acc,
	_arg_t_addr11,
	_arg_t_addr16,
	_arg_t_bit,
	_arg_t_dir,
	_arg_t_rDPTR,
	_arg_t_imm8,
	_arg_t_imm16,
	_arg_t_imm16be,
	_arg_t_bPSW_CY,
	_arg_t_rel,
	_arg_t_Rn,
	/* **** -- indirect */
	_arg_t_indirect = 1 << 8,
	_arg_t_atA_DPTRc,
	_arg_t_atDPTRx,
	_arg_t_atRi,
	/* **** a5 -- 16 bit operations */
	_arg_t_x16 = 1 << 9,
	_arg_t_WRx,			/* WRx */
	_arg_t_WRx_WR,		/* x, y -- x<<2 | y -- WRx, WRy */
	_arg_t_WR_WRy,
	_arg_t_WRx_iWR,		/* x, y -- x<<2 | y&f -- WRx, iWRy */
	_arg_t_WR_iWRy,
	/* **** -- implied */
//	_arg_t_implied_v = 1 << 31,
}arg_type;

#define ARG_T(_x) _arg_t_##_x
#define ARG_V(_x) (ARG_T(implied_v) | (_x << 8))

arg_p arg_src(vm_p vm, arg_type x);
arg_p arg_dst(vm_p vm, arg_type x);
void arg_wb(vm_p vm, arg_p x, uint32_t data);
