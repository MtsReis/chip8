CFLAGS=-Wall -Wextra -Werror -lSDL2

chip8: dir
	gcc src/main.c src/renderer.c src/chip8.c src/event.c -o bin/chip8 $(CFLAGS)

dir:
	mkdir -p bin