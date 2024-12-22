SRC=$(wildcard *.c)
OBJ=$(patsubst %.c,%.o,$(SRC))

CFLAGS=-Werror -std=c89 -g

all: libcommon.a

libcommon.a: $(OBJ)
	ar rcs $@ $^

test:
	$(MAKE) -C tests

clean:
	rm -f $(OBJ) ghostpp
	$(MAKE) -C tests $@
