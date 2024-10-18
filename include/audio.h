#ifndef _AUDIO_H
#define _AUDIO_H

#include <SDL2/SDL.h>
#include <stdbool.h>

bool audio_init(double sine_frequency);
void audio_play();
void audio_stop();
void audio_destroy();

#endif
