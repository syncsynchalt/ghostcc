SRC=$(wildcard *.c)
OBJ=$(filter-out y.tab.o, $(patsubst %.c,%.o,$(SRC))) y.tab.o

CFLAGS=-Werror -std=gnu89 -g -I../lib
LDFLAGS=-L../lib -lcommon -lm

all: ghostcc

test: all
	$(MAKE) -C tests test

../lib/libcommon.a:
	$(MAKE) -C ../lib libcommon.a

ghostcc: $(OBJ) ../lib/libcommon.a
	cc -o $@ $^ $(LDFLAGS)

y.tab.c: cc_grammar.y
	yacc $^
	yacc -d $^

clean:
	rm -f *.o ghostcc y.tab.h y.tab.c a.out
	$(MAKE) -C tests $@
