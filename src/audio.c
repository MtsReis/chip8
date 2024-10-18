#include "../include/audio.h"

// Define M_PI in case it's not already defined in math.h
#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

// Attributes for describing the sound samples
typedef struct
{
    double sineFreq;
    double sampleRate;
    double samplesPerSineCycle;
    double amplitude;
    uint32_t samplePos;
} Sound;

Sound sound;
SDL_AudioDeviceID audio_device;

void SDLAudioCallback(void *data, Uint8 *buffer, int length);

bool audio_init(double sine_frequency)
{
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        SDL_Log("Failed to initialize SDL audio subsystem. %s\n", SDL_GetError());
        return false;
    }

    // Set the values to used for calculating each sample
    sound.sineFreq = sine_frequency;
    sound.sampleRate = 44100;
    sound.samplesPerSineCycle = sound.sampleRate / sound.sineFreq;
    sound.samplePos = 0;
    sound.amplitude = 127.5;

    SDL_AudioSpec desiredSpec, obtainedSpec;

    SDL_zero(desiredSpec); // Clear memory block
    desiredSpec.freq = sound.sampleRate; // Samples per second
    desiredSpec.format = AUDIO_U8; // Unsigned 8-bit samples
    desiredSpec.channels = 1; // Mono
    desiredSpec.samples = 2048; // Buffer size
    desiredSpec.callback = SDLAudioCallback; // Callback that will feed the audio device
    desiredSpec.userdata = &sound; // Data to be used by the callback function. userdata is used to calculate the samples.

    // Try to open the most reasonable default device for playback
    audio_device = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, &obtainedSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (audio_device == 0)
    {
        printf("Failed to open audio: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

// Feed the playback buffer
void SDLAudioCallback(void *data, Uint8 *buffer, int length)
{
    Sound *sound = (Sound *)(data); // Convert data to Sound type

    // Calculate each sample value according to attributes in "data" (Sound struct), describing a sine wave
    for (int i = 0; i < length; ++i)
    {
        buffer[i] = (sin(sound->samplePos / sound->samplesPerSineCycle * M_PI * 2) + 1) * sound->amplitude;
        ++sound->samplePos;
    }
}

void audio_play()
{
    SDL_PauseAudioDevice(audio_device, 0);
}

void audio_stop()
{
    SDL_PauseAudioDevice(audio_device, 1);
}

void audio_destroy()
{
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}
