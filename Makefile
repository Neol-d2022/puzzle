CC=gcc

CFLAGS=-Wall -Wextra -Ofast
LFLAGS=-s

OBJS=avl.o dbank.o input.o worker.o core.o desicion.o main.o pq.o util.o
DEPS=avl.h dbank.h input.h worker.h core.h desicion.h main.h pq.h util.h
LIBS=-lpthread

BIN=puzzle

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJS) $(BIN)
