#ifndef SCENEDEFINITIONS_H
#define SCENEDEFINITIONS_H

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "Config.h"
#include "SceneDefinition.h"

enum class SceneId
{
    Basic,
    Alternate,
};

struct SceneCycleEntry
{
    SceneId id;
    float durationSeconds;
};

/**
 * @brief Loader and cache for scene configuration JSON files.
 */
class SceneDefinitions
{
public:
    /** @brief Get ordered default scene cycle from global config. */
    static const std::vector<SceneCycleEntry> &getDefaultSceneCycle();
    /** @brief Build scene definition for scene id if available. */
    static bool tryCreateSceneDefinition(SceneId id, SceneDefinition &outDefinition);
    /** @brief Get UI overlay configuration. */
    static const UIOverlayConfig &getUIOverlayConfig();
    /** @brief Get window mode and size configuration. */
    static const WindowConfig &getWindowConfig();

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
    static CameraMode parseCameraMode(const std::string &value);
    static WindowMode parseWindowMode(const std::string &value);
    static Object::VertexLayout parseVertexLayout(const std::string &value);
    static glm::vec3 parseVec3(float x, float y, float z);
    static SceneDefinition parseSceneDefinition(const std::string &sceneFilePath);
    static UIOverlayConfig parseUIOverlayConfig(const nlohmann::json &json);
    static WindowConfig parseWindowConfig(const nlohmann::json &json);
    static void ensureLoaded();

    /** Scene cycle entries parsed from scene_config.json. */
    static std::vector<SceneCycleEntry> sceneCycle;
    /** Parsed per-scene definitions keyed by SceneId integer value. */
    static std::unordered_map<int, SceneDefinition> sceneDefinitions;
    static UIOverlayConfig uiOverlayConfig;
    static WindowConfig windowConfig;
    static bool loaded;

    static const std::unordered_map<std::string, SceneId> sceneIdMap;
    static const std::unordered_map<std::string, RenderMode> renderModeMap;
    static const std::unordered_map<std::string, SceneRole> sceneRoleMap;
    static const std::unordered_map<std::string, BehaviorType> behaviorTypeMap;
    static const std::unordered_map<std::string, CameraMode> cameraModeMap;
    static const std::unordered_map<std::string, WindowMode> windowModeMap;
    static const std::unordered_map<std::string, Object::VertexLayout> vertexLayoutMap;
};

#endif // SCENEDEFINITIONS_H
