CFLAGS = -Wall -Wextra -ggdb
SOURCE_LIST = $(shell ls src/*)

SOURCE = src/main.c
TARGET = build/pancake
CC = gcc
 
all: build

build: $(SOURCE_LIST)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)
