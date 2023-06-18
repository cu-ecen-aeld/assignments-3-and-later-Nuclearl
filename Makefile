CC := gcc
CFLAGS := -Wall -Wextra -std=c11
LDFLAGS := -l syslog

TARGET := writer

ifeq ($(CROSS_COMPILE),aarch64-none-linux-gnu-)
	CC := $(CROSS_COMPILE)gcc
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): writer.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

writer.o: finder-app/writer.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o
