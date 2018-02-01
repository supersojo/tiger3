TARGET = main
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

.PHONY: all deps clean

all: deps $(TARGET)

$(TARGET): $(DEPS) $(OBJS) $(SRCS)
	@g++ $(OBJS) -o $@
	@echo linking $(TARGET) 

deps: $(DEPS)

%.d: %.c
	@g++ -MM $< > $@

%.o: %.c
	@g++ -c -g -Wno-write-strings $< -o $@
	@echo compiling $<

-include $(DEPS)

clean:
	rm -f main
	rm -f *.d
	rm -f *.o
