caso: *.c
	gcc -Wall -g -o caso caso.c text.c gfx.c touch.c test.c

debug: *.c
	gcc -DDEBUG_SERIAL -Wall -g -o caso caso.c text.c gfx.c touch.c test.c

all: caso
