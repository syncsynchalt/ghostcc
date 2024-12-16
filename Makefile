all: clean hello test

hello: hello.s
	as $^
	ld -o $@ $@.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path`

test: test.s
	as $^
	ld -o $@ $@.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path`
	./test

test2: test2.s
	as $^
	ld -o $@ $@.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path`
	./test2

PROGS=hello test test2
clean:
	rm -f $(PROGS) *.o

.PHONY: all clean $(PROGS)
