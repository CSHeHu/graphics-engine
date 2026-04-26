#ifndef SCENE_PLAYLIST_H
#define SCENE_PLAYLIST_H

#include <cstddef>
#include <vector>

#include "SceneDefinition.h"
#include "SceneDefinitions.h"

struct SceneTimelinePosition
{
    std::size_t index;
    float       sceneStartTimeSeconds;
};

/** @brief Owns configured scene cycle state and timeline resolution. */
class ScenePlaylist
{
  public:
    /** @brief Initialize playlist from configured scene cycle. */
    bool initialize(const std::vector<SceneCycleEntry>& configuredCycle);

    /** @brief Resolve active scene index and scene-local start time. */
    SceneTimelinePosition resolve(float timelineTimeSeconds) const;

    /** @brief Return scene id located at a resolved cycle index. */
    SceneId sceneIdAt(std::size_t index) const;

    /** @brief Check whether resolved timeline points to a different scene. */
    bool needsSwitch(const SceneTimelinePosition& timelinePosition) const;

    /** @brief Commit resolved timeline as active playlist state. */
    void commit(const SceneTimelinePosition& timelinePosition);

    /** @brief Get active scene id. */
    SceneId activeSceneId() const;

    /** @brief Get active scene-local start time. */
    float activeSceneStartTimeSeconds() const;

  private:
    std::vector<SceneCycleEntry> cycle;
    std::size_t                  position           = 0;
    SceneId                      activeSceneIdValue = SceneId::Basic;
    float                        activeSceneStartTimeSecondsValue = 0.0f;
};

#endif // SCENE_PLAYLIST_H