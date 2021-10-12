#define IR_Ri_READ() indirect_read(vm, IR_Ri)
#define IR_Ri_WRITE(_v) indirect_write(vm, IR_Ri, _v)

#define _Rn_(_vm, _r) \
	_vm->iram[(PSW & (3 << 3)) | (_r & 7)]

#define _IR_Ri_(_vm) _Rn_(_vm, IR_Ri)
#define _IR_Rn_(_vm) _Rn_(_vm, IR_Rn)
