SRC=$(wildcard *.c)
OBJ=$(filter-out y.tab.o, $(patsubst %.c,%.o,$(SRC))) y.tab.o

CFLAGS=-Werror -std=gnu89 -g -I../lib
LDFLAGS=-L../lib -lcommon -lm

all: ghostpp

test: all
	$(MAKE) -C tests test

ghostpp: $(OBJ) ../lib/libcommon.a
	cc -o $@ $^ $(LDFLAGS)

y.tab.c: pp_grammar.y
	yacc $^
	yacc -d $^

clean:
	rm -f *.o ghostpp y.tab.h y.tab.c
	$(MAKE) -C tests $@
