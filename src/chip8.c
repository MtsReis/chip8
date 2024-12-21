#include "../include/chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define PROGRAM_SECTION 512

// 60Hz
#define TIMER_REGISTERS_TIMESTEP 1.0 / 60.0

// 700Hz by default
double processor_timestep = 1.0 / 700.0;

// Time passed since latest cycle for dt and st
double tTimerRegistersFrequency = 0;

// Time passed since the latest instruction execution
double tProcessorFrequency = 0;

// Predefined sprites (5 bytes long each), from 0 to F
const char SPRITES[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
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

// Define whether the PC should advance to the next operation after execution
bool increasePC = true;

// Copy the content of a file in the specified path to the CHIP8 memory
bool chip8_loadGame(Chip8 *chip8, char *filePath)
{
    FILE *fp; // File Pointer
    int rel_address = PROGRAM_SECTION; // Section dedicated for the program

    printf("Loading file '%s'\n", filePath);

    // Check if file exists and if user has permission to read it
    if (access(filePath, F_OK) == -1)
    {
        fprintf(stderr, "File '%s' does not exist: %s\n", filePath, strerror(errno));
        return false;
    }

    if (access(filePath, R_OK) == -1)
    {
        fprintf(stderr, "User does not have permission to read the file '%s': %s\n", filePath, strerror(errno));
        return false;
    }

    // Open file for reading
    fp = fopen(filePath, "r");

    if (fp == NULL) {
        fprintf(stderr, "Failed to open file '%s'.", filePath);
        return false;
    }

    char currChar = fgetc(fp);

    // Read all chars until the end of file is reached
    while (!feof(fp))
    {
        chip8->memory[rel_address] = currChar; // Copy current byte
        rel_address += sizeof(currChar);       // Move the pointer to the next free address
        currChar = fgetc(fp);
    }

    printf("File loaded successfully.\n");

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
        fprintf(stderr, "Failed to decode opCode [0x%X]", opCode);
        return false;
    }

    // The PC should advance after running an opCode, unless the operation uncheck this
    increasePC = true;

    c->drawFlag = false;

    // Run the appropriate function based on the highestNibble
    bool success = (*decodeByHighestNibble[highestNibble])(opCode, c);

    if (!success)
        fprintf(stderr, "PC: %d | Invalid opCode: 0x%X", c->PC, opCode);

    // Advance the pointer to the next opcode (2 bytes) if increasePC is true
    c->PC += increasePC ? 2 : 0;

    return success;
}

bool chip8_emulateCycle(Chip8 *chip8, double deltaTime)
{
    // Update time passed since the lastest cycle for dt and st
    tTimerRegistersFrequency += deltaTime;

    // Update time passed since the lastest instruction execution
    tProcessorFrequency += deltaTime;

    if (tTimerRegistersFrequency >= TIMER_REGISTERS_TIMESTEP)
    { // Cycle completed
        // Reset the time passed and keep the surplus
        tTimerRegistersFrequency = tTimerRegistersFrequency - TIMER_REGISTERS_TIMESTEP;

        // Decrease dt and st by 1 to a minimum of 0
        chip8->dt = (chip8->dt - 1 > 0) ? chip8->dt - 1 : 0;
        chip8->st = (chip8->st - 1 > 0) ? chip8->st - 1 : 0;
    }

    if (tProcessorFrequency >= processor_timestep)
    { // Cycle completed
        tProcessorFrequency = tProcessorFrequency - processor_timestep; // Reset and keep the surplus

        return chip8_runInstruction(chip8);
    }

    return true;
}

// 0nnn
bool nib0(unsigned short opCode, Chip8 *c)
{
    switch (opCode & 0x00FF)
    {
    case 0x00E0: // CLS - Clear the display
        memset(c->gfx, false, sizeof(c->gfx));

        c->drawFlag = true;
        break;

    case 0x00EE: // RET - Return from a subroutine
        if (c->SP <= 0) {
            fprintf(stderr, "Stack underflow\n");
            return false;
        }

        c->SP--;
        c->PC = c->stack[c->SP];
        break;

    default:
        return false;
    }

    return true;
}

// 1nnn | JP addr - Jump to location nnn
bool nib1(unsigned short opCode, Chip8 *c)
{
    c->PC = opCode & 0x0FFF;

    increasePC = false;
    return true;
}

// 2nnn | CALL addr - Call subroutine at nnn
bool nib2(unsigned short opCode, Chip8 *c)
{
    if (c->SP >= 15) {
        fprintf(stderr, "Stack overflow\n");
        return false;
    }

    c->stack[c->SP] = c->PC;
    c->SP++;
    c->PC = opCode & 0x0FFF;

    increasePC = false;
    return true;
}

// 3xkk | SE Vx, byte - Skip next instruction if Vx = kk
bool nib3(unsigned short opCode, Chip8 *c)
{
    if (c->V[(opCode & 0x0F00) >> 8] == (opCode & 0x00FF))
        c->PC += 2;

    return true;
}

// 4xkk | SNE Vx, byte - Skip next instruction if Vx != kk
bool nib4(unsigned short opCode, Chip8 *c)
{
    if (c->V[(opCode & 0x0F00) >> 8] != (opCode & 0x00FF))
        c->PC += 2;

    return true;
}

// 5xy0 | SE Vx, Vy - Skip next instruction if Vx = Vy
bool nib5(unsigned short opCode, Chip8 *c)
{
    if (c->V[(opCode & 0x0F00) >> 8] == c->V[(opCode & 0x00F0) >> 4])
        c->PC += 2;

    return true;
}

// 6xkk | LD Vx, byte - Set Vx = kk
bool nib6(unsigned short opCode, Chip8 *c)
{
    c->V[(opCode & 0x0F00) >> 8] = opCode & 0x00FF;

    return true;
}

// 7xkk | ADD Vx, byte - Set Vx = Vx + kk
bool nib7(unsigned short opCode, Chip8 *c)
{
    c->V[(opCode & 0x0F00) >> 8] += opCode & 0x00FF;

    return true;
}

// 8xyn
bool nib8(unsigned short opCode, Chip8 *c)
{
    // Every opcode from this set follows the format 8xyn
    char Vf;
    int x = (opCode & 0x0F00) >> 8;
    int y = (opCode & 0x00F0) >> 4;

    switch (opCode & 0x000F)
    {
    case 0x0000: // 8xy0 | LD Vx, Vy - Set Vx = Vy
        c->V[x] = c->V[y];
        break;

    case 0x0001: // 8xy1 | OR Vx, Vy - Set Vx = Vx OR Vy
        c->V[x] |= c->V[y];
        break;

    case 0x0002: // 8xy2 | AND Vx, Vy - Set Vx = Vx AND Vy
        c->V[x] &= c->V[y];
        break;

    case 0x0003: // 8xy3 | XOR Vx, Vy - Set Vx = Vx XOR Vy
        c->V[x] ^= c->V[y];
        break;

    case 0x0004: // 8xy4 | ADD Vx, Vy - Set Vx = Vx + Vy, set VF = carry
        Vf = ((int)(c->V[x] + c->V[y]) > 0xFF);
        c->V[x] = (c->V[x] + c->V[y]) & 0xFF; // Save only 1 byte from the result
        c->V[0xF] = Vf;
        break;

    case 0x0005: // 8xy5 | SUB Vx, Vy - Set Vx = Vx - Vy, set VF = NOT borrow
        Vf = (c->V[x] >= c->V[y]); // No underflow
        c->V[x] = (c->V[x] - c->V[y]) & 0xFF; // Save only 1 byte from the result
        c->V[0xF] = Vf;
        break;

    case 0x0006: // 8xy6 | SHR Vx {, Vy} - Set Vx = Vx SHR 1
        if (!c->shiftQuirk) // This quirk makes so Y is ignored for this operation
            c->V[x] = c->V[y];

        Vf = c->V[x] % 2; // if Vx is odd, Vf = 1 | if Vx is even, Vf = 0
        c->V[x] >>= 1; // Vx/2
        c->V[0xF] = Vf;
        break;

    case 0x0007: // 8xy7 | SUBN Vx, Vy - Set Vx = Vy - Vx, set VF = NOT borrow
        Vf = (c->V[y] >= c->V[x]); // No underflow
        c->V[x] = c->V[y] - c->V[x];
        c->V[0xF] = Vf;
        break;

    case 0x000E: // 8xyE | SHL Vx {, Vy} - Set Vx = Vx SHL 1
        if (!c->shiftQuirk) // This quirk makes so Y is ignored for this operation
            c->V[x] = c->V[y];

        Vf = (c->V[x] & 128) >> 7; // 128 = (10000000)₂
        c->V[x] <<= 1; // Vx * 2
        c->V[0xF] = Vf;
        break;

    default:
        return false;
    }

    return true;
}

// 9xy0 | SNE Vx, Vy - Skip next instruction if Vx != Vy
bool nib9(unsigned short opCode, Chip8 *c)
{
    if (c->V[(opCode & 0x0F00) >> 8] != c->V[(opCode & 0x00F0) >> 4])
        c->PC += 2;

    return true;
}

// Annn | LD I, addr - Set I = nnn
bool nibA(unsigned short opCode, Chip8 *c)
{
    c->I = opCode & 0x0FFF;

    return true;
}

// Bnnn | JP V0, addr - Jump to location nnn + V0
bool nibB(unsigned short opCode, Chip8 *c)
{
    c->PC = c->V[0] + (opCode & 0x0FFF);

    increasePC = false;
    return true;
}

// Cxkk | RND Vx, byte - Set Vx = random byte AND kk
bool nibC(unsigned short opCode, Chip8 *c)
{
    c->V[(opCode & 0x0F00) >> 8] = (rand() % 256) & (opCode & 0x00FF);
    return true;
}

// Dxyn | DRW Vx, Vy, nibble - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
bool nibD(unsigned short opCode, Chip8 *c)
{
    unsigned char wishX = c->V[(opCode & 0x0F00) >> 8];
    unsigned char wishY = c->V[(opCode & 0x00F0) >> 4];
    unsigned char height = opCode & 0x000F;
    unsigned char sprByte; // Each byte represents the whole row of pixels
    unsigned short finalX, finalY;

    // No collision by default
    c->V[0xF] = 0;

    for (int line = 0; line < height; line++)
    {
        // (wishY + line) % CHIP8_GFX_H for a Y value inside the CHIP8_GFX_H limits
        finalY = (wishY + line) % CHIP8_GFX_H * CHIP8_GFX_W;

        sprByte = c->memory[c->I + line];
        for(int column = 0; column < 8; column++)
        {
            /* Check if the bit (in sprByte) at position defined by 'column',
             * from left to right (0b10000000 >> column), is not 0
             */
            if((sprByte & (0b10000000 >> column)) != 0) {
                // Set the X position, inside the CHIP8_GFX_W limits, for the spr pixel
                finalX = (wishX + column) % CHIP8_GFX_W; // 1

                // Collision
                if(c->gfx[finalY + finalX])
                    c->V[0xF] = 1;

                c->gfx[finalY + finalX] ^= 1;
            }
        }
    }

    c->drawFlag = true;

    return true;
}

// Ennn
bool nibE(unsigned short opCode, Chip8 *c)
{
    switch (opCode & 0x00FF)
    {
    case 0x009E: // Ex9E | SKP Vx - Skip next instruction if key with the value of Vx is pressed
        if (c->key[ c->V[(opCode & 0x0F00) >> 8] ])
            c->PC += 2;
        break;

    case 0x00A1: // ExA1 | SKNP Vx - Skip next instruction if key with the value of Vx is not pressed
        if (!c->key[ c->V[(opCode & 0x0F00) >> 8] ])
            c->PC += 2;
        break;

    default:
        return false;
    }

    return true;
}

// Fnnn
bool nibF(unsigned short opCode, Chip8 *c)
{
    switch (opCode & 0x00FF)
    {
    case 0x0007: // Fx07 | LD Vx, DT - Set Vx = delay timer value
        c->V[(opCode & 0x0F00) >> 8] = c->dt;
        break;

    case 0x000A: // Fx0A | LD Vx, K - Wait for a key press, store the value of the key in Vx
        for(int i=0; i<16; i++) {
            if (c->key[i]) {
                c->V[(opCode & 0x0F00) >> 8] = i;
                return true;
            }
        }

        // Repeat instruction if none of the keys are being pressed
        increasePC = false;
        break;

    case 0x0015: // Fx15 | LD DT, Vx - Set delay timer = Vx
        c->dt = c->V[(opCode & 0x0F00) >> 8];
        break;

    case 0x0018: // Fx18 | LD ST, Vx - Set sound timer = Vx
        c->st = c->V[(opCode & 0x0F00) >> 8];
        break;

    case 0x001E: // Fx1E | ADD I, Vx - Set I = I + Vx
        c->I += c->V[(opCode & 0x0F00) >> 8];
        break;

    case 0x0029: // Fx29 | LD F, Vx - Set I = location of sprite for digit Vx
        c->I = c->V[(opCode & 0x0F00) >> 8] * 5;
        break;

    case 0x0033: // Fx33 | LD B, Vx - Store BCD representation of Vx in memory locations I, I+1, and I+2
        c->memory[c->I]     = c->V[(opCode & 0x0F00) >> 8] / 100;
        c->memory[c->I + 1] = (c->V[(opCode & 0x0F00) >> 8] / 10) % 10;
        c->memory[c->I + 2] = (c->V[(opCode & 0x0F00) >> 8] % 100) % 10;
        break;

    case 0x0055: // Fx55 | LD [I], Vx - Store registers V0 through Vx in memory starting at location I
        for(int i=0; i <= (opCode & 0x0F00) >> 8; i++) {
            c->memory[c->I + i] = c->V[i];
        }
        break;

    case 0x0065: // Fx65 | LD Vx, [I] - Read registers V0 through Vx from memory starting at location I
        for (int i = 0; i <= (opCode & 0x0F00) >> 8; i++) {
            c->V[i] = c->memory[c->I + i];
        }
        break;

    default:
        return false;
    }

    return true;
}

void chip8_init(Chip8 *chip8, unsigned int processor_freq)
{
    // Clear memory
    memset(chip8->memory, 0, sizeof(chip8->memory));

    // Clear registers
    memset(chip8->V, 0, sizeof(chip8->V));

    // Clear special registers
    chip8->I = 0;
    chip8->PC = PROGRAM_SECTION;
    chip8->SP = 0;
    chip8->dt = 0;
    chip8->st = 0;
    chip8->drawFlag = false;

    // Quirks activated by default
    chip8->shiftQuirk = true;

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

    processor_timestep = 1.0 / processor_freq;
}
