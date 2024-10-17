#ifndef _CHIP8_H
#define _CHIP8_H

#include <stdbool.h>

#define CHIP8_GFX_W 64
#define CHIP8_GFX_H 32

typedef struct
{
    // 4,096 bytes of memory
    unsigned char memory[4096];

    // 16 8-bits registers
    unsigned char V[18];

    // Special registers
    unsigned short I;  // Memory addresses
    unsigned short PC; // Program Counter
    unsigned char SP;  // Stack Pointer
    unsigned char dt;  // Delay timer
    unsigned char st;  // Sound Timer

    unsigned short stack[16];

    bool gfx[CHIP8_GFX_W * CHIP8_GFX_H];

    unsigned char drawFlag;
} Chip8;

// Open and read file with given [directory/]filename. Return whether it succeeded or not
bool chip8_loadGame(char *file);
void chip8_emulateCycle();

#endif