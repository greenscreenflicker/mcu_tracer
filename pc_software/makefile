CC=gcc
CFLAGS=-I./ -O3 `pkg-config --cflags gtk+-3.0`
DEPS = hellomake.h
COMPILEFILES = main.o connect.o interfacer.o rs232.o debugwindow.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
hellomake: $(COMPILEFILES)
	$(CC) -o mcu_tracker $(COMPILEFILES) `pkg-config --libs gtk+-3.0`
