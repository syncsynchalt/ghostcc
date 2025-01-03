SRC=$(wildcard *.cc)
OBJ=$(patsubst %.cc,%.o,$(SRC))
APP_OBJ=$(filter-out ../cpp.o,$(wildcard ../*.o))

CXXFLAGS=-w -g -I/opt/homebrew/include -I.. -I../../lib -std=c++14
LDFLAGS=-L/opt/homebrew/lib -lgtest -lgtest_main -L../../lib -lcommon

all: test

test: tests run

tests: $(OBJ) $(APP_OBJ)
	echo $(APP_OBJ)
	c++ $(LDFLAGS) -o tests $^

run: tests
	./tests

clean:
	rm -f *.o tests
