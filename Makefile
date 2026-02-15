CFLAGS = -std=c11 -g -Wall
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) $(CFLAGS) -o 9cc $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS): 9cc.h

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean
