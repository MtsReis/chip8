#include "../include/chip8.h"

#include <stdio.h>
#include <unistd.h>

bool chip8_loadGame(char *file)
{
    FILE *fp;

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
        printf("%c\n", currChar); // Currently doing nothing with the read byte ;_;
        currChar = fgetc(fp);
    }

    return true;
}

void chip8_emulateCycle()
{
}
