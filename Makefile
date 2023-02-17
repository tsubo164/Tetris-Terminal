CC      := g++
OPT     := -g
CFLAGS  := $(OPT) -Wall --pedantic-errors --std=c++14 -c
LDFLAGS := -lncurses
RM      := rm -f

SRCS    := field main piece tetris tetromino

.PHONY: clean

TETRIS  := tetris
OBJS := $(addsuffix .o, $(SRCS))
DEPS := $(addsuffix .d, $(SRCS))

all: $(TETRIS)

$(OBJS): %.o: %.cc
	$(CC) $(CFLAGS) -o $@ $<

$(TETRIS): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(TETRIS) *.o *.d

$(DEPS): %.d: %.cc
	$(CC) -c -MM $< > $@

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPS)
endif
