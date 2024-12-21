SRC=$(wildcard *.c)
OBJ=$(patsubst %.c,%.o,$(SRC))

CFLAGS=-std=c89 -g

ghostpp: $(OBJ)
	cc -o $@ $^

clean:
	rm -f $(OBJ) ghostpp
