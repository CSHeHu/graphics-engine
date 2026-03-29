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

SceneDefinition createBasicSceneDefinition();
SceneDefinition createAlternateSceneDefinition();

const std::vector<SceneRegistryEntry> &getSceneRegistry();
bool tryCreateSceneDefinition(SceneId id, SceneDefinition &outDefinition);

#endif // SCENEDEFINITIONS_H
