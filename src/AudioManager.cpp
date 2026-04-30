#include "AudioManager.h"
#include <SDL_mixer.h>

AudioManager::AudioManager()
{
}

AudioManager::~AudioManager()
{
    Mix_CloseAudio();
    SDL_Quit();
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
        return -1;
    }
    return 0;
}

std::shared_ptr<Mix_Music> AudioManager::load(const char* file)
{
    Mix_Music* music = Mix_LoadMUS(file);
    if (music == NULL)
    {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    }

    return std::shared_ptr<Mix_Music>(music, Mix_FreeMusic);
}
int AudioManager::play(const std::shared_ptr<Mix_Music> music, int loops)
{

    int isPlaying = Mix_PlayMusic(music.get(), loops);
    if (isPlaying != 0)
    {
        printf("Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
        return -1;
    }

    return 0;
}
