CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lssl -lcrypto -lpthread

# List of source files
SRCS = tls_server.c https_methods.c ssl_queue.c thread_pool.c

# Header file
HDRS = https_methods.h

# Derive object file names from source files
OBJS = $(SRCS:.c=.o)

# Target executable
TARGET = tls_server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
