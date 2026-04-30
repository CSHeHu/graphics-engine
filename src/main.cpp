#include "Application.h"
#include "AudioManager.h"
#include <SDL_mixer.h>
#include <iostream>
#include <thread>

int main()
{
    Application app;
    if (!app.init())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    AudioManager audioManager;
    if (audioManager.init() != 0)
    {
        std::cout << "Failed to initialize audio manager" << std::endl;
        return -1;
    }

    std::shared_ptr<Mix_Music> babydoll =
        audioManager.load("assets/audio/babydoll.mp3");
    std::thread audio([&]() { audioManager.play(babydoll); });
    app.run();
    audio.join();

    return 0;
}
