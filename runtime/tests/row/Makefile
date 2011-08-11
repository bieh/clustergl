CC = gcc -Wall -ansi

all:
	$(CC) row.c -o row -lGL -lGLU `sdl-config --cflags --libs` --std=c99

clean:
	@rm row -f
