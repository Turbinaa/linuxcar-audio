PROJ = linuxcar-audio

CC = gcc
CFLAGS = -Wall -Wextra -O3
INCLUDE = -I/usr/include/spa-0.2/ -I/usr/include/pipewire-0.3/
LDFLAGS = -lpipewire-0.3 -lm -pthread -lzmq

EXE = build/$(PROJ)

all: $(EXE)

$(EXE): src/*.c src/*/*.c | build
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS)

clean:
	rm $(EXE)

build:
	mkdir -p build

.PHONY: all clean

