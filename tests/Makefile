CC      := g++
OPT     := -g
CFLAGS  := $(OPT) -Wall --pedantic-errors --std=c++14 -c -I..
LDFLAGS := -lncurses
RM      := rm -f

SRCS    := $(filter-out ../main.cc, $(wildcard ../*.cc))

.PHONY: clean test

TETRIS  := ../tetris
TEST_MAIN := test_main
OBJS := $(patsubst %.cc, %.o, $(SRCS))

all: test

test: $(TEST_MAIN)
	./$(TEST_MAIN)
	@echo "\033[0;32mOK\033[0;39m"

$(TEST_MAIN): $(TETRIS) test.o
	$(CC) -o $@ $(OBJS) test.o $(LDFLAGS)

$(TETRIS):
	$(MAKE) -C ../

%.o: %.cc
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(TEST_MAIN) *.o
