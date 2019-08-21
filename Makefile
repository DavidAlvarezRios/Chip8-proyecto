FILES_C = chip8.c
TARGET = chip8
CFLAGS = -Wall -g -O0
CC = gcc
CC_SDL = `sdl2-config --cflags --libs`

FILES_O = $(subst .c,.o,$(FILES_C))

$(TARGET) : $(FILES_O) Makefile
		$(CC) $(FILES_O) $(CC_SDL) -o $(TARGET)

%.o: %.c Makefile
		$(CC) $(CFLAGS) -c $<

all: $(TARGET)

clean:
		/bin/rm $(FILES_O) $(TARGET)
