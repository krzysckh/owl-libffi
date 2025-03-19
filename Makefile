OSOURCE=/home/kpm/nmojeprogramy/owl

test.c: test.scm
	ol -x c -o test.c test.scm
test: test.c
	# -lsqlite3 or other libraries might be needed on custom ol builds
	cc -ggdb -I$(OSOURCE)/c -DPRIM_FP_API -DPRIM_CUSTOM test.c ffi.c `pkg-config --cflags --libs libffi` -ldl -o test
clean:
	rm -f test test.c
