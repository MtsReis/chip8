#ifndef _RENDERER_H
#define _RENDERER_H

#include <stdbool.h>

bool gfx_init(int w, int h);
void gfx_draw(bool gfx[]);
void gfx_destroy();

#endif
