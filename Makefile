CC=gcc

CFLAGS=-Wall -Wextra -Ofast
LFLAGS=-s

OBJS=avl.o dbank.o input.o worker.o core.o desicion.o dijkstra.o main.o plates.o pq.o util.o xy.o
DEPS=avl.h dbank.h input.h worker.h core.h desicion.h dijkstra.h main.h plates.h pq.h util.h xy.h
LIBS=-lpthread -lm

BIN=puzzle

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJS) $(BIN)
