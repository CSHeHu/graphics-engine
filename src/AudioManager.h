#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <memory>

/**
 * @brief Manages audio playback and initialization using SDL2 and SDL_mixer.
 */
class AudioManager
{
    public:
        AudioManager();
        ~AudioManager();
        /**
         * @brief Initialize SDL audio and SDL_mixer subsystems.
         * @return 0 on success, -1 on failure.
         */
        int init();
        /**
         * @brief Play a music track.
         * @param music Shared pointer to Mix_Music object.
         * @param loops Number of times to loop (-1 for infinite).
         * @return 0 on success, -1 on failure.
         */
        int play(const std::shared_ptr<Mix_Music>& music, int loops = -1);
        /**
         * @brief Stop music playback.
         */
        void stop();
        /**
         * @brief Pause music playback.
         */
        void pause();
        /**
         * @brief Resume music playback.
         */
        void resume();
        /**
         * @brief Set playback position in seconds.
         * @param seconds Position in seconds.
         * @return 0 on success, -1 on failure.
         */
        int setPosition(double seconds);

    private:
        /**
         * @brief Indicates whether the audio system has been initialized.
         */
        bool initialized = false;
};

#endif // !AUDIOMANAGER_H
