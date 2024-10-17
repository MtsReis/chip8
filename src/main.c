#include "../include/chip8.h"
#include "../include/renderer.h"
#include "../include/event.h"

int main()
{
    Chip8 chip8;
    bool halt_execution = false;

    // Try to load rom and initialize subsystems: exit on failure
    if (!chip8_loadGame("idk.ch8") || !gfx_init(CHIP8_GFX_W, CHIP8_GFX_H) || !event_init())
        exit(EXIT_FAILURE);

    // Emulation loop
    while (true)
    {
        event_update(chip8.key, &halt_execution);

        // Clean up initialized subsystems on a quit event
        if (halt_execution)
        {
            printf("\nShutting down");
            gfx_destroy();
            event_destroy();
            printf("\nBye bye!\n");
            exit(EXIT_SUCCESS);
        }

        chip8_emulateCycle();

        if (chip8.drawFlag)
            gfx_draw(chip8.gfx);
    }
}
