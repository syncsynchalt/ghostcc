SRC=$(wildcard *.c)
OBJ=$(patsubst %.c,%.o,$(SRC))

CFLAGS=-Werror -std=c89 -g

all: libcommon.a

libcommon.a: $(OBJ)
	ar rcs $@ $^

test: all
	$(MAKE) -C tests

clean:
	rm -f *.o ghostpp
	$(MAKE) -C tests $@
