PROJ = audio-vis

CC = gcc
CFLAGS = -Wall -Wextra -O3 -Wno-unused-variable -Wno-unused-parameter
#-fsanitize=address
INCLUDE = -I/usr/include/spa-0.2/ -I/usr/include/pipewire-0.3/
LDFLAGS = -lpipewire-0.3 -lm -pthread

EXE= $(PROJ)

all: $(EXE)

$(EXE): src/*.c src/*/*.c
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LDFLAGS)

clean:
	rm $(EXE)

.PHONY: all clean

