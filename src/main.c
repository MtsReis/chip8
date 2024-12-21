#include <time.h>

#include "../include/chip8.h"
#include "../include/renderer.h"
#include "../include/event.h"
#include "../include/audio.h"

#include <SDL2/SDL.h>

int main(int argc, char*argv[])
{
    Chip8 chip8;

    clock_t time = clock();
    double deltaTime = 0;
    unsigned int processor_freq = 700;

    bool halt_execution = false;

    if (argc != 2) {
        fprintf(stderr, "One argument is expected.\n");
        exit(EXIT_FAILURE);
    }

    chip8_init(&chip8, processor_freq);

    // Try to load rom and initialize subsystems: exit on failure
    if (!chip8_loadGame(&chip8, argv[1]) || !gfx_init(CHIP8_GFX_W, CHIP8_GFX_H) || !event_init() || !audio_init(1337))
        exit(EXIT_FAILURE);

    // Emulation loop
    while (true)
    {
        event_update(chip8.key, &halt_execution);

        // Clean up initialized subsystems on a quit event
        if (halt_execution)
        {
            printf("\nHalting execution");

            gfx_destroy();
            event_destroy();
            audio_destroy();

            printf("\nBye bye!\n");
            exit(EXIT_SUCCESS);
        }

        // Update at what time the cycle is being executed and how much has passed since last cycle (s)
        deltaTime = (double)(clock() - time) / CLOCKS_PER_SEC;
        time = clock();
        halt_execution = !chip8_emulateCycle(&chip8, deltaTime);

        if (chip8.PC >= sizeof(chip8.memory)) {
            fprintf(stderr, "PC exceeded the memory limits.");
            halt_execution = true;
            continue;
        }

        if (chip8.st > 0) {
            audio_play();
        } else {
            audio_stop();
        }

        if (chip8.drawFlag)
            gfx_draw(chip8.gfx);
    }
}
