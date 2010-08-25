CC = gcc -Wall -ansi

all:
	$(CC) lesson12.c -o lesson12 -lGL -lGLU `sdl-config --cflags --libs` --std=c99

clean:
	@echo Cleaning up...
	@rm lesson12
	@echo Done.
