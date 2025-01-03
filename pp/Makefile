SRC=$(wildcard *.c)
OBJ=$(patsubst %.c,%.o,$(SRC)) y.tab.o

CFLAGS=-Werror -std=c89 -g -I../lib
LDFLAGS=-L../lib -lcommon

all: ghostpp

test:
	$(MAKE) -C tests test

ghostpp: $(OBJ)
	cc $(LDFLAGS) -o $@ $^

y.tab.c: pp_grammar.y
	yacc $^
	yacc -d $^

clean:
	rm -f *.o ghostpp y.tab.h y.tab.c
	$(MAKE) -C tests $@
