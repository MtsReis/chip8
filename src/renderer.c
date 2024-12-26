#include <SDL2/SDL.h>

#include "../include/renderer.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int gfx_w;
int gfx_h;
unsigned char bg[3];
unsigned char fg[3];

bool gfx_init(int w, int h, unsigned char bg_colour[3], unsigned char fg_colour[3])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Failed to initialize SDL video subsystem. %s\n", SDL_GetError());
        return false;
    }

    gfx_w = w;
    gfx_h = h;
    memcpy(bg, bg_colour, sizeof(unsigned char) * 3);
    memcpy(fg, fg_colour, sizeof(unsigned char) * 3);

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_DisplayMode DM;
    SDL_GetDesktopDisplayMode(0, &DM);

    SDL_CreateWindowAndRenderer(gfx_w, gfx_h, 0, &window, &renderer);
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

    SDL_RenderSetScale(renderer, (float)DM.w / gfx_w, (float)DM.h / gfx_h);

    return true;
}

void gfx_draw(bool gfx[])
{
    SDL_SetRenderDrawColor(renderer, bg[0], bg[1], bg[2], 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, fg[0], fg[1], fg[2], 255);

    for (int i = 0; i < gfx_w; i++)
    {
        for (int j = 0; j < gfx_h; j++)
        {
            if (gfx[i + gfx_w * j])
            {
                SDL_RenderDrawPoint(renderer, i, j);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void gfx_destroy()
{
    SDL_Quit();
}
