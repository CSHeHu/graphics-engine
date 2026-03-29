#ifndef SCENEDEFINITIONS_H
#define SCENEDEFINITIONS_H

#include <vector>

#include "SceneDefinition.h"

enum class SceneId
{
    Basic,
    Alternate,
};

struct SceneRegistryEntry
{
    SceneId id;
    SceneDefinition (*factory)();
};

struct SceneCycleEntry
{
    SceneId id;
    float durationSeconds;
};

SceneDefinition createBasicSceneDefinition();
SceneDefinition createAlternateSceneDefinition();

const std::vector<SceneRegistryEntry> &getSceneRegistry();
const std::vector<SceneCycleEntry> &getDefaultSceneCycle();
bool tryCreateSceneDefinition(SceneId id, SceneDefinition &outDefinition);

#endif // SCENEDEFINITIONS_H
