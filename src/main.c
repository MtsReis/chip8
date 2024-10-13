#include <SDL2/SDL.h>

#include "../include/chip8.h"
#include "../include/renderer.h"

SDL_Event e;

int main()
{
    Chip8 chip8;

    // Try to load rom and initialize subsystems: exit on failure
    if (!chip8_loadGame("idk.ch8") || !gfx_init(CHIP8_GFX_W, CHIP8_GFX_H))
        exit(EXIT_FAILURE);

    // Emulation loop
    while (true)
    {
        // Loop through SDL events
        while (SDL_PollEvent(&e))
        {
            // Clean up initialized subsystems on a quit event
            if (e.type == SDL_QUIT)
            {
                gfx_destroy();
                exit(EXIT_SUCCESS);
            }
        }

        chip8_emulateCycle();

        if (chip8.drawFlag)
            gfx_draw(chip8.gfx);
    }
}
