#include <SDL2/SDL.h>

#include "../include/renderer.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int gfx_w;
int gfx_h;

bool sdl_init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        SDL_Log("Failed to initialize SDL. %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool gfx_init(int w, int h)
{
    if (sdl_init())
    {
        gfx_w = w;
        gfx_h = h;

        SDL_Init(SDL_INIT_EVERYTHING);

        SDL_DisplayMode DM;
        SDL_GetDesktopDisplayMode(0, &DM);

        SDL_CreateWindowAndRenderer(gfx_w, gfx_h, 0, &window, &renderer);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

        SDL_RenderSetScale(renderer, (float)DM.w / gfx_w, (float)DM.h / gfx_h);

        return true;
    }

    return false;
}

void gfx_draw(bool gfx[])
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

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
