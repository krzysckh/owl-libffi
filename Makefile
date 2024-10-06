OSOURCE=/home/kpm/nmojeprogramy/owl

test.c: test.scm
	ol -x c -o test.c test.scm
test: test.c
	cc -ggdb -I$(OSOURCE)/c -DPRIM_FP_API -DPRIM_CUSTOM test.c ffi.c -lffi -ldl -o test
clean:
	rm -f test test.c
