#include "AudioManager.h"

AudioManager::AudioManager()
{
}

AudioManager::~AudioManager()
{
}

void AudioManager::run()
{

    SDL_Init(SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    Mix_Music* music = Mix_LoadMUS("TMP");
    Mix_PlayMusic(music, 1);

    SDL_Delay(50000); // wait while it plays

    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();
}
