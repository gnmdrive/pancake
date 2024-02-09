CFLAGS = -Wall -Wextra
SOURCE = src/main.c
TARGET = build/pancake
CC = gcc
 
all: build

build: $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)
