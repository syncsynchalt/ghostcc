SRC=$(wildcard *.c)
OBJ=$(patsubst %.c,%.o,$(SRC))

CFLAGS=-Werror -std=gnu89 -g

all: libcommon.a

libcommon.a: $(OBJ)
	ar rcs $@ $^

test: all
	$(MAKE) -C tests

clean:
	rm -f *.o libcommon.a a.out
	$(MAKE) -C tests $@
