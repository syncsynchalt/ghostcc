SRC=$(wildcard *.c)
OBJ=$(filter-out y.tab.o, $(patsubst %.c,%.o,$(SRC))) y.tab.o

CFLAGS=-Werror -std=c89 -g -I../lib
LDFLAGS=-L../lib -lcommon

all: ghostcc

test: all
	$(MAKE) -C tests test

ghostcc: $(OBJ) ../lib/libcommon.a
	cc $(LDFLAGS) -o $@ $(OBJ)

y.tab.c: cc_grammar.y
	yacc $^
	yacc -d $^

clean:
	rm -f *.o ghostcc y.tab.h y.tab.c
	# $(MAKE) -C tests $@
