#include "ScenePlaylist.h"

bool ScenePlaylist::initialize(
    const std::vector<SceneCycleEntry>& configuredCycle)
{
    cycle = configuredCycle;
    if (cycle.empty())
    {
        return false;
    }

    position                         = 0;
    activeSceneIdValue               = cycle[position].id;
    activeSceneStartTimeSecondsValue = 0.0f;
    return true;
}

SceneTimelinePosition ScenePlaylist::resolve(float timelineTimeSeconds) const
{
    if (cycle.empty())
    {
        return {0, 0.0f};
    }

    float accumulatedStartTime = 0.0f;
    for (std::size_t i = 0; i < cycle.size(); ++i)
    {
        const bool  isLastScene     = (i + 1 == cycle.size());
        const float durationSeconds = cycle[i].durationSeconds;

        if (isLastScene)
        {
            return {i, accumulatedStartTime};
        }

        if (durationSeconds <= 0.0f)
        {
            return {i, accumulatedStartTime};
        }

        const float nextSceneStartTime = accumulatedStartTime + durationSeconds;
        if (timelineTimeSeconds < nextSceneStartTime)
        {
            return {i, accumulatedStartTime};
        }

        accumulatedStartTime = nextSceneStartTime;
    }

    return {cycle.size() - 1, accumulatedStartTime};
}

SceneId ScenePlaylist::sceneIdAt(std::size_t index) const
{
    return cycle[index].id;
}

bool ScenePlaylist::needsSwitch(
    const SceneTimelinePosition& timelinePosition) const
{
    return timelinePosition.index != position;
}

void ScenePlaylist::commit(const SceneTimelinePosition& timelinePosition)
{
    position                         = timelinePosition.index;
    activeSceneIdValue               = cycle[position].id;
    activeSceneStartTimeSecondsValue = timelinePosition.sceneStartTimeSeconds;
}

SceneId ScenePlaylist::activeSceneId() const
{
    return activeSceneIdValue;
}

float ScenePlaylist::activeSceneStartTimeSeconds() const
{
    return activeSceneStartTimeSecondsValue;
}