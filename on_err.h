#define ON_ERR(_fn, _expect, _test) \
	({ \
		if(_expect == (_test)) { \
			perror(#_test); \
			goto _fn; \
		}\
	})

#define ON_0ERR(_fn, _test) ON_ERR(_fn, 0, _test)
#define ON_1ERR(_fn, _test) ON_ERR(_fn, -1, _test)
