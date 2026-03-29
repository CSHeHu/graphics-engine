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
    std::string filePath;
};

struct SceneCycleEntry
{
    SceneId id;
    float durationSeconds;
};

const std::vector<SceneRegistryEntry> &getSceneRegistry();
const std::vector<SceneCycleEntry> &getDefaultSceneCycle();
bool tryCreateSceneDefinition(SceneId id, SceneDefinition &outDefinition);

#endif // SCENEDEFINITIONS_H
