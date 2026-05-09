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
        currentMusic.reset();
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

    currentMusic  = music;
    int isPlaying = Mix_PlayMusic(music.get(), loops);
    if (isPlaying != 0)
    {
        printf("Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
        currentMusic.reset();
        return -1;
    }

    return 0;
}

void AudioManager::stop()
{
    if (initialized)
    {
        Mix_HaltMusic();
        currentMusic.reset();
    }
}

void AudioManager::pause()
{
    if (initialized)
    {
        Mix_PauseMusic();
    }
}

void AudioManager::resume()
{
    if (initialized)
    {
        Mix_ResumeMusic();
    }
}

int AudioManager::setPosition(double seconds)
{
    if (!initialized)
    {
        return -1;
    }

    // Mix_SetMusicPosition returns 0 on success, -1 on error.
    int res = Mix_SetMusicPosition(seconds);
    if (res != 0)
    {
        printf("Failed to set music position: %s\n", Mix_GetError());
        return -1;
    }
    return 0;
}
