#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <memory>

class AudioManager
{
    public:
        AudioManager();
        ~AudioManager();
        int  init();
        int  play(const std::shared_ptr<Mix_Music>& music, int loops = -1);
        void stop();

    private:
        bool initialized = false;
};

#endif // !AUDIOMANAGER_H
