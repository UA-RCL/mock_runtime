INCLUDES = ../libdash
RUNTIME_LIBDIR = ../build/libdash
STANDALONE_LIBDIR = ../libdash/build

all: test_app.so test_app.out

test_app.so:
	$(CC) -shared -I $(INCLUDES) test_app.cpp -o test_app.so

# Open issue: attempts to statically link libm with the rest of libdash have failed. So when linking against libdash.a, linking libm is also required
test_app.out:
	$(CC) -I $(INCLUDES) -L $(STANDALONE_LIBDIR) test_app.cpp -l:libdash.a -lm -o test_app.out

clean:
	-rm test_app.so
	-rm test_app.out
