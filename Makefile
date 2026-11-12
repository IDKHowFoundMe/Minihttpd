CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE -Iinclude

SRC_DIR = src
TARGET = minihttp

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/server.c $(SRC_DIR)/http.c $(SRC_DIR)/files.c $(SRC_DIR)/mime.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)


rebuild: clean all

.PHONY: all clean rebuild
