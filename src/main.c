#include <time.h>
#include <limits.h>
#include <math.h>

#include "../include/chip8.h"
#include "../include/renderer.h"
#include "../include/event.h"
#include "../include/audio.h"

#include <SDL2/SDL.h>

// Extract the RGB values from a string that follows the format "#RRGGBB"
bool parseRGB(const char *str, unsigned char channel[3]);

int main(int argc, char *argv[])
{
    // DIR is a required argument
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s DIR [--freq <int>] [--sound <double>] [--bg \"#RRGGBB\"] [--fg \"#RRGGBB\"]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    clock_t time = clock();
    double deltaTime = 0;

    bool halt_execution = false;

    Chip8 chip8;
    char *romDir = NULL;
    int processor_freq = 700;
    double sound_freq = 264;
    unsigned char bg_colour[3] = {0, 0, 0};
    unsigned char fg_colour[3] = {255, 255, 255};

    // Arguments validation
    for (int i = 1; i < argc; i++)
    {
        // [--freq <int>]
        if (strcmp(argv[i], "--freq") == 0)
        {
            if (i + 1 < argc)
            {
                char *arg = argv[i + 1];

                // If the arg isn't in the signed int value range, set the freq to unrestricted (-1)
                processor_freq = atoi(arg) <= INT_MAX && atoi(arg) >= INT_MIN ? atoi(arg) : -1;

                // Fail only if an invalid input was given to atoi (atoi returning 0 when that wasn't the desired value)
                if (processor_freq != 0 || strcmp(arg, "0") == 0)
                {
                    i++; // Skip the next argument
                    continue;
                }
            }

            // Error if the requeriments weren't met
            fprintf(stderr, "Error: --freq requires a valid integer value.\n");
            exit(EXIT_FAILURE);
        }

        // [--sound <double>]
        if (strcmp(argv[i], "--sound") == 0)
        {
            if (i + 1 < argc)
            {
                // Pointer to keep track of where strtod stops converting
                char *endptr;

                // Convert the arg value to a double
                sound_freq = strtod(argv[i + 1], &endptr);

                // Check if strtod() was able to convert the entire string and it isn't an special value
                if (*endptr == '\0' && !isinf(sound_freq) && !isnan(sound_freq))
                {
                    i++; // Skip the next argument
                    continue;
                }
            }

            // Error if the requeriments weren't met
            fprintf(stderr, "Error: --sound requires a valid double value.\n");
            exit(EXIT_FAILURE);
        }

        // [--bg <#RRGGBB>]
        if (strcmp(argv[i], "--bg") == 0)
        {
            if (i + 1 < argc)
            {
                if (parseRGB(argv[i + 1], bg_colour))
                {
                    i++; // Skip the next argument
                    continue;
                }
            }

            // Error if the requeriments weren't met
            fprintf(stderr, "Error: --bg requires a valid hex RGB value in the format #RRGGBB.\n");
            exit(EXIT_FAILURE);
        }

        // [--fg <#RRGGBB>]
        if (strcmp(argv[i], "--fg") == 0)
        {
            if (i + 1 < argc)
            {
                if (parseRGB(argv[i + 1], fg_colour))
                {
                    i++; // Skip the next argument
                    continue;
                }
            }

            // Error if the requeriments weren't met
            fprintf(stderr, "Error: --fg requires a valid hex RGB value in the format #RRGGBB.\n");
            exit(EXIT_FAILURE);
        }

        // Handle rom directory
        if (romDir != NULL)
        {
            printf("A rom directory was already provided. Ignoring argument: %s\n", argv[i]);
            continue;
        }

        romDir = argv[i];
    }

    // Ensure a directory was provided
    if (romDir == NULL)
    {
        fprintf(stderr, "Error: A directory must be provided.\n");
        exit(EXIT_FAILURE);
    }

    chip8_init(&chip8, processor_freq);

    // Try to load rom and initialize subsystems: exit on failure
    if (!chip8_loadGame(&chip8, romDir) || !gfx_init(CHIP8_GFX_W, CHIP8_GFX_H, bg_colour, fg_colour) || !event_init() || !audio_init(sound_freq))
        exit(EXIT_FAILURE);

    // Emulation loop
    while (true)
    {
        event_update(chip8.key, &halt_execution);

        // Clean up initialized subsystems on a quit event
        if (halt_execution)
        {
            printf("\nHalting execution");

            gfx_destroy();
            event_destroy();
            audio_destroy();

            printf("\nBye bye!\n");
            exit(EXIT_SUCCESS);
        }

        // Update at what time the cycle is being executed and how much has passed since last cycle (s)
        deltaTime = (double)(clock() - time) / CLOCKS_PER_SEC;
        time = clock();
        halt_execution = !chip8_emulateCycle(&chip8, deltaTime);

        if (chip8.PC >= sizeof(chip8.memory))
        {
            fprintf(stderr, "Error: PC exceeded the memory limits.");
            halt_execution = true;
            continue;
        }

        if (chip8.st > 0)
        {
            audio_play();
        }
        else
        {
            audio_stop();
        }

        if (chip8.drawFlag)
            gfx_draw(chip8.gfx);
    }
}

bool parseRGB(const char *str, unsigned char channel[3])
{
    // Check if it starts with # and it has the appropriate length
    if (str[0] != '#' || strlen(str) != 7)
    {
        return false;
    }

    // Check if the other chars are valid hex digits
    for (int i = 1; i < 7; i++)
    {
        if (!isxdigit(str[i]))
        {
            return false;
        }
    }

    // Extract the RGB values
    if (sscanf(str + 1, "%2hhx%2hhx%2hhx", &channel[0], &channel[1], &channel[2]) != 3)
    {
        fprintf(stderr, "Error: Failed to parse the RGB values from '%s'.\n", str);
        return false;
    }

    return true;
}
