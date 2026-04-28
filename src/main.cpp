#include "Application.h"
#include "AudioManager.h"
#include <iostream>
#include <thread>

int main()
{
    Application  app;
    AudioManager audioManager;
    if (!app.init())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }

    std::thread audio([&]() { audioManager.run(); });
    app.run();
    audio.join();

    return 0;
}
