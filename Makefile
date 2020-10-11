CC=gcc
CFLAGS=-ggdb3 -Wall -Wextra -fsanitize=undefined
LIBS=-lcrypto -lev -luwsc -ljansson -lcurl
OBJECTS=build/main.o build/cord.o build/discord.o build/http.o
TARGET=bin/cord

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $(TARGET)

build/main.o: src/main.c
	$(CC) -c $(CFLAGS) src/main.c $(LIBS) -o build/main.o

build/cord.o: src/cord.c src/cord.h
	$(CC) -c $(CFLAGS) src/cord.c $(LIBS) -o build/cord.o

build/discord.o: src/discord.c src/discord.h
	$(CC) -c $(CFLAGS) src/discord.c $(LIBS) -o build/discord.o

build/http.o: src/http.c src/http.h
	$(CC) -c $(CFLAGS) src/http.c -lcurl -o build/http.o



clean:
	rm -f build/*
	rm -f bin/*
