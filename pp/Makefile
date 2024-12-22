SRC=$(wildcard *.c)
OBJ=$(patsubst %.c,%.o,$(SRC))

CFLAGS=-Werror -std=c89 -g -I../lib
LDFLAGS=-L../lib -lcommon

all: ghostpp

test:
	$(MAKE) -C tests test

ghostpp: $(OBJ)
	cc $(LDFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) ghostpp
	$(MAKE) -C tests $@
