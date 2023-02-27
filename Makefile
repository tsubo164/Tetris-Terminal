CC      := g++
OPT     := -g
CFLAGS  := $(OPT) -Wall --pedantic-errors --std=c++14 -c
LDFLAGS := -lncurses
RM      := rm -f

SRCS    := display field main piece scorer tetris tetromino

.PHONY: clean test

TETRIS  := tetris
OBJS := $(addsuffix .o, $(SRCS))
DEPS := $(addsuffix .d, $(SRCS))

all: $(TETRIS)

$(OBJS): %.o: %.cc
	$(CC) $(CFLAGS) -o $@ $<

$(TETRIS): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# TESTS ======================
TEST_OBJS = $(filter-out main.o, $(OBJS)) test.o

test_main: $(TEST_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

test: test_main
	./test_main
	@echo "\033[0;32mOK\033[0;39m"

test.o: test.cc
	$(CC) $(CFLAGS) -o $@ $<
#TESTS ======================

clean:
	$(RM) $(TETRIS) test_main *.o *.d

$(DEPS): %.d: %.cc
	$(CC) -c -MM $< > $@

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPS)
endif
