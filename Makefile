CC=gcc
CFLAGS=-ggdb3 -Wall -Wextra -fPIC -fsanitize=undefined
LIBS=-lcrypto -lev -luwsc -ljansson -lcurl
OBJECTS=build/cord.o build/discord.o build/http.o
TARGET=bin/libcord.so

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -shared $(OBJECTS) $(LIBS) -o $(TARGET)

build/cord.o: src/cord.c src/cord.h
	$(CC) -c $(CFLAGS) src/cord.c $(LIBS) -o build/cord.o

build/discord.o: src/discord.c src/discord.h
	$(CC) -c $(CFLAGS) src/discord.c $(LIBS) -o build/discord.o

build/http.o: src/http.c src/http.h
	$(CC) -c $(CFLAGS) src/http.c -lcurl -o build/http.o


clean:
	rm -f build/*
	rm -f bin/*

install:
	cp bin/libcord.so /usr/lib/libcord.so
	cp src/cord.h /usr/local/include/cord.h
	cp src/discord.h /usr/local/include/discord.h
	cp src/http.h /usr/local/include/http.h

uninstall:
	rm -f /usr/local/lib/libcord.so

