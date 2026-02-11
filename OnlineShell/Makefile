CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -g
LDFLAGS :=

TARGETS := server client

.PHONY: all clean run-server run-client

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS) *.o

# Convenience targets (not required, but nice)
run-server: server
	./server $(PORT)

run-client: client
	./client $(HOST) $(PORT)