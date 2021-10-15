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
	_arg_t_atA_DPTR,
	_arg_t_atDPTR,
	_arg_t_atRi,
	_arg_t_bit,
	_arg_t_dir,
	_arg_t_rDPTR,
	_arg_t_rDPX,
	_arg_t_imm8,
	_arg_t_imm16,
	_arg_t_imm16be,
	_arg_t_bPSW_CY,
	_arg_t_rel,
	_arg_t_Rn,
	/* **** a5 -- 16 bit operations */
	_arg_t_x2,
	_arg_t_X2y2,
	_arg_t_x2Y2,
	/* **** -- implied */
	_arg_t_implied_v = 0x81,
}arg_type;

#define ARG_T(_x) _arg_t_##_x
#define ARG_V(_x) (ARG_T(implied_v) | (_x << 8))

arg_p arg_src(vm_p vm, arg_type x);
arg_p arg_dst(vm_p vm, arg_type x);
void arg_wb(vm_p vm, arg_p x, uint32_t data);
