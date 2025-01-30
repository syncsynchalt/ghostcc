SRC=$(wildcard *.cc)
OBJ=$(patsubst %.cc,%.o,$(SRC))

CXXFLAGS=-w -g -I/opt/homebrew/include -std=c++14
LDFLAGS=-L/opt/homebrew/lib -lgtest -lgtest_main -L.. -lcommon

all: test

test: tests run

tests: $(OBJ) ../libcommon.a
	c++ -o tests $(OBJ) $(LDFLAGS)

run: tests
	./tests

clean:
	rm -f *.o tests a.out
