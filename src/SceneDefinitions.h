#ifndef SCENEDEFINITIONS_H
#define SCENEDEFINITIONS_H

#include <fstream>
#include <string>
#include <unordered_map>
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

class SceneDefinitions
{
public:
    static const std::vector<SceneRegistryEntry> &getSceneRegistry();
    static const std::vector<SceneCycleEntry> &getDefaultSceneCycle();
    static bool tryCreateSceneDefinition(SceneId id, SceneDefinition &outDefinition);

private:
    template <typename T>
    static T parseEnumValue(const std::string &value,
                            const std::unordered_map<std::string, T> &mapping,
                            const char *typeName);

    static std::ifstream openAssetFile(const std::string &path);
    static SceneId parseSceneId(const std::string &value);
    static RenderMode parseRenderMode(const std::string &value);
    static SceneRole parseSceneRole(const std::string &value);
    static BehaviorType parseBehaviorType(const std::string &value);
    static Object::VertexLayout parseVertexLayout(const std::string &value);
    static glm::vec3 parseVec3(float x, float y, float z);
    static SceneDefinition parseSceneDefinition(const std::string &sceneFilePath);
    static void ensureLoaded();

    static std::vector<SceneRegistryEntry> sceneRegistry;
    static std::vector<SceneCycleEntry> sceneCycle;
    static std::unordered_map<int, SceneDefinition> sceneDefinitions;
    static bool loaded;

    static const std::unordered_map<std::string, SceneId> sceneIdMap;
    static const std::unordered_map<std::string, RenderMode> renderModeMap;
    static const std::unordered_map<std::string, SceneRole> sceneRoleMap;
    static const std::unordered_map<std::string, BehaviorType> behaviorTypeMap;
    static const std::unordered_map<std::string, Object::VertexLayout> vertexLayoutMap;
};

#endif // SCENEDEFINITIONS_H
