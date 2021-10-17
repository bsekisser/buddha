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

#define _PSW(_x)					BEXT(PSW, PSW_BIT_##_x)
#define PSW_CY						_PSW(CY)
