SRC=$(wildcard *.cc)
OBJ=$(patsubst %.cc,%.o,$(SRC))
APP_OBJ=$(filter-out ../pp.o,$(wildcard ../*.o))

CXXFLAGS=-w -g -I/opt/homebrew/include -I.. -I../../lib -std=c++14
LDFLAGS=-L/opt/homebrew/lib -lgtest -lgtest_main -L../../lib -lcommon

all: test

test: tests run

tests: $(OBJ) $(APP_OBJ) ../../lib/libcommon.a
	c++ -o tests $(OBJ) $(APP_OBJ) $(LDFLAGS)


run: tests
	./tests

clean:
	rm -f *.o tests
