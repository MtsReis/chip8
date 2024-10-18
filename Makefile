CFLAGS=-Wall -Wextra -Werror -lSDL2 -lm

chip8: dir
	gcc src/main.c src/renderer.c src/chip8.c src/event.c src/audio.c -o bin/chip8 $(CFLAGS)

dir:
	mkdir -p bin