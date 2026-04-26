#ifndef TIME_STATE_H
#define TIME_STATE_H

/** @brief Owns simulation time, delta time, and pause state. */
class TimeState
{
  public:
    /** @brief Reset simulation and delta timing from current real time. */
    void initialize(float nowRealTimeSeconds);

    /** @brief Compute non-negative real-time delta and advance real clock. */
    float computeRealDelta(float nowRealTimeSeconds);

    /** @brief Step simulation time forward by a fixed amount. */
    void stepForward(float stepSeconds);

    /** @brief Step simulation time backward, clamping to zero. */
    void stepBackward(float stepSeconds);

    /** @brief Advance simulation when unpaused and update frame delta state. */
    void advance(float realDeltaSeconds);

    /** @brief Get current simulation time. */
    float currentTimeSeconds() const;

    /** @brief Get frame delta time. */
    float deltaTimeSeconds() const;

    /** @brief Check whether simulation is paused. */
    bool isPaused() const;

    /** @brief Update pause state. */
    void setPaused(bool value);

  private:
    float currentTimeSecondsValue  = 0.0f;
    float deltaTimeSecondsValue    = 0.0f;
    float lastRealTimeSecondsValue = 0.0f;
    bool  pausedValue              = false;
};

#endif // TIME_STATE_H