#ifndef SCENECONFIGLOADER_H
#define SCENECONFIGLOADER_H

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "SceneDefinition.h"
#include "VertexLayout.h"

enum class SceneId
{
    Basic,
    Alternate,
};

struct SceneCycleEntry
{
        SceneId id;
        float   durationSeconds;
};

/**
 * @brief Loader and cache for scene configuration JSON files.
 */
class SceneConfigLoader
{
    public:
        SceneConfigLoader() = default;

        /** @brief Get ordered default scene cycle from config. */
        const std::vector<SceneCycleEntry>& getDefaultSceneCycle();
        /** @brief Build scene definition for scene id if available. */
        bool tryCreateSceneDefinition(
            SceneId id, std::shared_ptr<SceneDefinition>& outDefinition);
        /** @brief Get UI overlay configuration. */
        const UIOverlayConfig& getUIOverlayConfig();
        /** @brief Get window mode and size configuration. */
        const WindowConfig& getWindowConfig();
        /** @brief Get runtime configuration loaded from scene_config.json. */
        const RuntimeConfig& getRuntimeConfig();

    private:
        template <typename T>
        T parseEnumValue(const std::string&                        value,
                         const std::unordered_map<std::string, T>& mapping,
                         const char* typeName) const;

        std::ifstream   openAssetFile(const std::string& path);
        SceneId         parseSceneId(const std::string& value);
        RenderMode      parseRenderMode(const std::string& value);
        SceneRole       parseSceneRole(const std::string& value);
        BehaviorType    parseBehaviorType(const std::string& value);
        CameraMode      parseCameraMode(const std::string& value);
        WindowMode      parseWindowMode(const std::string& value);
        VertexLayout    parseVertexLayout(const std::string& value);
        glm::vec3       parseVec3(float x, float y, float z);
        SceneDefinition parseSceneDefinition(const std::string& sceneFilePath);
        UIOverlayConfig parseUIOverlayConfig(const nlohmann::json& json);
        WindowConfig    parseWindowConfig(const nlohmann::json& json);
        RuntimeConfig   parseRuntimeConfig(const nlohmann::json& json);
        void            ensureLoaded();

        /** Scene cycle entries parsed from scene_config.json. */
        std::vector<SceneCycleEntry> sceneCycle;
        /** Parsed per-scene definitions keyed by SceneId integer value. */
        std::unordered_map<int, std::shared_ptr<SceneDefinition>>
                        sceneDefinitions;
        UIOverlayConfig uiOverlayConfig;
        WindowConfig    windowConfig;
        RuntimeConfig   runtimeConfig;
        bool            loaded = false;

        const std::unordered_map<std::string, SceneId> sceneIdMap = {
            {"Basic", SceneId::Basic},
            {"Alternate", SceneId::Alternate},
        };
        const std::unordered_map<std::string, RenderMode> renderModeMap = {
            {"LightSource", RenderMode::LightSource},
            {"Lit", RenderMode::Lit},
        };
        const std::unordered_map<std::string, SceneRole> sceneRoleMap = {
            {"None", SceneRole::None},
            {"LightSource", SceneRole::LightSource},
            {"LightTarget", SceneRole::LightTarget}};
        const std::unordered_map<std::string, BehaviorType> behaviorTypeMap = {
            {"None", BehaviorType::None},
            {"Oscillate", BehaviorType::Oscillate},
            {"Spin", BehaviorType::Spin},
            {"Fly", BehaviorType::Fly},
        };
        const std::unordered_map<std::string, CameraMode> cameraModeMap = {
            {"Manual", CameraMode::Manual},
            {"Scripted", CameraMode::Scripted},
        };
        const std::unordered_map<std::string, WindowMode> windowModeMap = {
            {"Windowed", WindowMode::Windowed},
            {"Fullscreen", WindowMode::Fullscreen},
        };
        const std::unordered_map<std::string, VertexLayout> vertexLayoutMap = {
            {"PositionNormal", VertexLayout::PositionNormal},
        };
};

#endif // SCENECONFIGLOADER_H
