LIBS = -lm
CFLAGS = -Wall -Wextra -ggdb
SOURCE_LIST = $(shell ls src/*)

SOURCE = src/main.c
TARGET = bin/pancake
 
all: build

build: $(SOURCE_LIST)
	gcc $(CFLAGS) $(SOURCE) -o $(TARGET) $(LIBS)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)
