#include "AudioManager.h"
#include <SDL_mixer.h>
#include <cstdio>

AudioManager::AudioManager()
{
}

AudioManager::~AudioManager()
{
    if (initialized)
    {
        Mix_HaltMusic();
        Mix_CloseAudio();
        Mix_Quit();
        SDL_Quit();
    }
}

int AudioManager::init()
{

    int isInit = SDL_Init(SDL_INIT_AUDIO);
    if (isInit != 0)
    {
        printf("Failed to initialize SDL! SDL Error: %s\n", SDL_GetError());
        return -1;
    }
    isInit = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    if (isInit != 0)
    {
        printf("Failed to initialize SDL_mixer! SDL_mixer Error: %s\n",
               Mix_GetError());
        SDL_Quit();
        return -1;
    }
    initialized = true;
    return 0;
}

int AudioManager::play(const std::shared_ptr<Mix_Music>& music, int loops)
{
    if (!initialized)
    {
        return -1;
    }

    if (!music)
    {
        printf("Failed to play music: null music handle\n");
        return -1;
    }

    int isPlaying = Mix_PlayMusic(music.get(), loops);
    if (isPlaying != 0)
    {
        printf("Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
        return -1;
    }

    return 0;
}

void AudioManager::stop()
{
    if (initialized)
    {
        Mix_HaltMusic();
    }
}
