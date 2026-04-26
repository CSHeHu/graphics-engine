#include "TimeState.h"

void TimeState::initialize(float nowRealTimeSeconds)
{
    currentTimeSecondsValue  = 0.0f;
    deltaTimeSecondsValue    = 0.0f;
    lastRealTimeSecondsValue = nowRealTimeSeconds;
    pausedValue              = false;
}

float TimeState::computeRealDelta(float nowRealTimeSeconds)
{
    float realDeltaSeconds = nowRealTimeSeconds - lastRealTimeSecondsValue;
    if (realDeltaSeconds < 0.0f)
    {
        realDeltaSeconds = 0.0f;
    }
    lastRealTimeSecondsValue = nowRealTimeSeconds;
    return realDeltaSeconds;
}

void TimeState::stepForward(float stepSeconds)
{
    currentTimeSecondsValue += stepSeconds;
}

void TimeState::stepBackward(float stepSeconds)
{
    currentTimeSecondsValue -= stepSeconds;
    if (currentTimeSecondsValue < 0.0f)
    {
        currentTimeSecondsValue = 0.0f;
    }
}

void TimeState::advance(float realDeltaSeconds)
{
    if (!pausedValue)
    {
        currentTimeSecondsValue += realDeltaSeconds;
        deltaTimeSecondsValue = realDeltaSeconds;
        return;
    }

    deltaTimeSecondsValue = 0.0f;
}

float TimeState::currentTimeSeconds() const
{
    return currentTimeSecondsValue;
}

float TimeState::deltaTimeSeconds() const
{
    return deltaTimeSecondsValue;
}

bool TimeState::isPaused() const
{
    return pausedValue;
}

void TimeState::setPaused(bool value)
{
    pausedValue = value;
}