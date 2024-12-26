#ifndef _RENDERER_H
#define _RENDERER_H

#include <stdbool.h>

bool gfx_init(int w, int h, unsigned char bg_colour[3], unsigned char fg_colour[3]);
void gfx_draw(bool gfx[]);
void gfx_destroy();

#endif
