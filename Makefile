FILES_C = chip8.c
TARGET = chip8
CFLAGS = -O -Wall
CC = gcc

FILES_O = $(subst .c,.o,$(FILES_C))

$(TARGET) : $(FILES_O) Makefile
		$(CC) $(FILES_O) -o $(TARGET)

%.o: %.c Makefile
		$(CC) $(CFLAGS) -c $<

all: $(TARGET)

clean:
		/bin/rm $(FILES_O) $(TARGET)
