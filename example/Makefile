CC := gcc

CFLAG := -I../include
FUS_LIB := -L../ -lfusion -Wl,-rpath,'$$ORIGIN'

all:
	$(CC) $(CFLAG) main.c $(FUS_LIB) -o exemple_main
	$(CC) $(CFLAG) program_exemple.c $(FUS_LIB) -o exemple_elf

clean:
	rm -f exemple_elf exemple_main