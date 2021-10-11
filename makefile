CFLGAS += -Wall -g -O2
#CFLAGS += --save-temps

OBJS = vm8051.o vm_test.o

vm: $(OBJS)
	cc -o vm $(OBJS)

vm_test.o: vm_test.c vm8051.h on_err.h

vm8051.o: vm8051.c vm8051.h on_err.h

clean:
	-rm vm *.o *.i *.s *.out
