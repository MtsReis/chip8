#include "../include/chip8.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

// 60Hz
#define TIMESTEP 1.0 / 60.0

// Time passed since latest cycle for dt and st
double freq_timer = 0;

// Predefined sprites (5 bytes long each), from 0 to F
const char SPRITES[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80,
};

/*
 * Store the addresses for all the functions used to decode an opCode.
 * For the sake of organization, there are 16 (0x0 to 0xF) distinct functions.
 * Each function has the logic for all possible opCodes that start with a
 * specific nibble.
 * E.g.: nibE() (decodeByHighestNibble[14]) decodes all opCodes starting with
 * 'E' (like E19E or EFA1).
 */
bool (*decodeByHighestNibble[16])(unsigned short, Chip8 *);

bool chip8_loadGame(Chip8 *chip8, char *file)
{
    FILE *fp;
    int rel_address = 512; // Section dedicated for the program

    // Check if file exists and if user has permission to read it
    if (access(file, F_OK) == -1)
    {
        printf("File does not exist.\n");
        return false;
    }

    if (access(file, R_OK) == -1)
    {
        printf("User does not have permission to read the file.\n");
        return false;
    }

    printf("File loaded successfully.\n");

    // Open file for reading
    fp = fopen(file, "r");

    char currChar = fgetc(fp);

    // Read all chars until the end of file is reached
    while (currChar != EOF)
    {
        chip8->memory[rel_address] = currChar; // Copy current byte
        rel_address += sizeof(currChar);       // Move the pointer to the next free address
        currChar = fgetc(fp);
    }

    return true;
}

bool chip8_runInstruction(Chip8 *c)
{
    // Merge the next 2 bytes (size of an opCode) into a 2 bytes-long data type (short)
    unsigned short opCode = c->memory[c->PC] << 8 | c->memory[c->PC + 1];

    /*
     * Acquire the highest nibble (4 bits, 1 hex digit) by ignoring the second half (1 byte)
     * of the opcode and discarding 4 bits from the first half (higher byte of the opcode)
     * NXXX -> The Xs (lower nibbles) are discarded and N is saved to identify which function
     * in decodeByHighestNibble[] to run.
     */
    unsigned short highestNibble = c->memory[c->PC] >> 4;

    if (highestNibble > 15)
    {
        printf("Failed to decode opCode [0x%X]", opCode);
        return false;
    }

    printf("S[%d] | ", highestNibble);

    // Run the appropriate function based on the highestNibble
    bool success = (*decodeByHighestNibble[highestNibble])(opCode, c);

    if (!success)
        printf("Invalid opCode: 0x%X", opCode);

    printf("\n");

    // Advance the pointer to the next opcode (2 bytes)
    c->PC += 2;

    return true;
}

void chip8_emulateCycle(Chip8 *chip8, double deltaTime)
{
    // Update time passed since the lastest cycle for dt and st
    freq_timer += deltaTime;

    if (freq_timer >= TIMESTEP)
    { // Cycle completed
        // Decrease dt and st by 1 to a minimum of 0
        chip8->dt = (chip8->dt - 1 > 0) ? chip8->dt - 1 : 0;
        chip8->st = (chip8->st - 1 > 0) ? chip8->st - 1 : 0;
        freq_timer = freq_timer - TIMESTEP; // Reset the time passed and keep the surplus
    }

    chip8_runInstruction(chip8);
}

// 0x0nnn
bool nib0(unsigned short opCode, Chip8 *c)
{
    switch (opCode & 0x00FF)
    {
    case 0x00E0: // CLS - Clear the display
        printf("PC: %d | 0x%X - CLS", c->PC, opCode);
        break;

    case 0x00EE: // RET - Return from a subroutine
        printf("PC: %d | 0x%X - RET", c->PC, opCode);
        break;

    default:
        return false;
    }

    return true;
}

// 0x1nnn | JP addr - Jump to location nnn
bool nib1(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - JP addr", c->PC, opCode);
    return true;
}

// 0x2nnn | CALL addr - Call subroutine at nnn
bool nib2(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - CALL addr", c->PC, opCode);
    return true;
}

// 0x3nnn | SE Vx, byte - Skip next instruction if Vx = kk
bool nib3(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - SE Vx, byte", c->PC, opCode);
    return true;
}

// 0x4nnn | SNE Vx, byte - Skip next instruction if Vx != kk
bool nib4(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - SNE Vx, byte", c->PC, opCode);
    return true;
}

// 0x5nnn | SE Vx, Vy - Skip next instruction if Vx = Vy
bool nib5(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - SE Vx, Vy", c->PC, opCode);
    return true;
}

// 0x6nnn | LD Vx, byte - Set Vx = kk
bool nib6(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - LD Vx, byte", c->PC, opCode);
    return true;
}

// 0x7nnn | ADD Vx, byte - Set Vx = Vx + kk
bool nib7(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - ADD Vx, byte", c->PC, opCode);
    return true;
}

// 0x8nnn
bool nib8(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X", c->PC, opCode);
    return true;
}

// 0x9nnn | SNE Vx, Vy - Skip next instruction if Vx != Vy
bool nib9(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - SNE Vx, Vy", c->PC, opCode);
    return true;
}

// 0xAnnn | LD I, addr - Set I = nnn
bool nibA(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - LD I, addr", c->PC, opCode);
    return true;
}

// 0xBnnn | JP V0, addr - Jump to location nnn + V0
bool nibB(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - JP V0, addr", c->PC, opCode);
    return true;
}

// 0xCnnn | RND Vx, byte - Set Vx = random byte AND kk
bool nibC(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - RND Vx, byte", c->PC, opCode);
    return true;
}

// 0xDnnn | DRW Vx, Vy, nibble - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
bool nibD(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X - DRW Vx, Vy, nibble", c->PC, opCode);
    return true;
}

// 0xEnnn
bool nibE(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X", c->PC, opCode);
    return true;
}

// 0xFnnn
bool nibF(unsigned short opCode, Chip8 *c)
{
    printf("PC: %d | 0x%X", c->PC, opCode);
    return true;
}

void chip8_init(Chip8 *chip8)
{
    // Clear memory
    memset(chip8->memory, 0, sizeof(chip8->memory));

    // Clear registers
    memset(chip8->V, 0, sizeof(chip8->V));

    // Clear special registers
    chip8->I = 0;
    chip8->PC = 512; // Program section
    chip8->SP = 0;
    chip8->dt = 0;
    chip8->st = 0;
    chip8->drawFlag = false;

    // Clear stack
    memset(chip8->stack, 0, sizeof(chip8->stack));

    // Clear display
    memset(chip8->gfx, false, sizeof(chip8->gfx));

    // Clear keypad
    memset(chip8->key, false, sizeof(chip8->key));

    // Copy the sprites to the interpreter area of memory
    memcpy(chip8->memory, SPRITES, sizeof(SPRITES));

    // Map decode functions to decodeByHighestNibble
    memcpy(
        decodeByHighestNibble,
        (bool (*[16])(unsigned short, Chip8 *)){
            &nib0, &nib1, &nib2, &nib3, &nib4, &nib5, &nib6, &nib7,
            &nib8, &nib9, &nibA, &nibB, &nibC, &nibD, &nibE, &nibF,
        },
        sizeof(decodeByHighestNibble)
    );
}
