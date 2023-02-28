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

test: $(TETRIS)
	$(MAKE) -C tests $@

clean:
	$(RM) $(TETRIS) *.o *.d
	$(MAKE) -C tests $@

$(DEPS): %.d: %.cc
	$(CC) -c -MM $< > $@

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPS)
endif
