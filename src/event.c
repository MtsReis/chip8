#include "../include/event.h"
#include <SDL2/SDL.h>

/*
    Map the SDL Keycodes to a CHIP-8 key from 0 to F
    Index: CHIP-8 Keypad
    Value: User keyboard
    +---------+---------+---------+---------+
    |1: SDLK_1|2: SDLK_2|3: SDLK_3|C: SDLK_4|
    +---------+---------+---------+---------+
    |4: SDLK_q|5: SDLK_w|6: SDLK_e|D: SDLK_r|
    +---------+---------+---------+---------+
    |7: SDLK_a|8: SDLK_s|9: SDLK_d|E: SDLK_f|
    +---------+---------+---------+---------+
    |A: SDLK_z|0: SDLK_x|B: SDLK_c|F: SDLK_v|
    +---------+---------+---------+---------+
*/
const SDL_Keycode keyMap[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v};

SDL_Event e;

bool event_init()
{
    if (SDL_Init(SDL_INIT_EVENTS) != 0)
    {
        SDL_Log("Failed to initialize SDL event subsystem. %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void event_update(bool keypad[16], bool *quit)
{
    // Loop through SDL events
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            *quit = true;

        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
        {
            // Check if any of the known/mapped keycodes matches the one being pressed/release
            for (int i = 0; i < 16; i++)
            {
                if (keyMap[i] == e.key.keysym.sym)
                {
                    // If it's a KEYDOWN event, stores true, otherwise, KEYUP is assumed and stores false
                    keypad[i] = e.type == SDL_KEYDOWN;
                }
            }
        }
    }
}

void event_destroy()
{
    SDL_Quit();
}
