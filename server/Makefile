CC=gcc
CFLAGS= -g -Wall
FL=aesdsocket aesdsocket.c

all: aesdsocket

aesdsocket: aesdsocket.c
	$(CC) -static $(CFLAGS) $(INCLUDES) $(OBJS) -o $(FL) $(LDFLAGS)

clean:
	rm -f aesdsocket *.o
