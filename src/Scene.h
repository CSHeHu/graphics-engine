#ifndef SCENE_H
#define SCENE_H

#include <array>
#include <cstddef>
#include <glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "SceneDefinition.h"

class Camera;
class Object;
class Shader;
class AssetManager;
class TextManager;

/** @brief Runtime material state resolved from scene configuration. */
struct RuntimeMaterial
{
    std::shared_ptr<Shader> shader;
    RenderMode              renderMode;
    glm::vec3               objectColor;
};

/** @brief Runtime object state used for rendering and behavior updates. */
struct RuntimeSceneObject
{
    std::shared_ptr<Object>          object;
    std::shared_ptr<RuntimeMaterial> material;
    std::size_t                      indexCount;
    SceneRole                        role;
    BehaviorType                     behavior;
    float                            behaviorSpeed;
    glm::vec3                        behaviorAxis;
    float                            behaviorAmplitude;
    glm::vec3                        initialPosition;
    float                            initialRotationAngle;
    glm::vec3                        lightColor;
    float                            lightIntensity;
};

/**
 * @brief Runtime scene container that updates behaviors and renders scene
 * content.
 */
class Scene
{
  public:
    /** @brief Construct scene runtime from parsed definition and shared
     * managers. */
    Scene(AssetManager&                    assetManager,
          std::shared_ptr<SceneDefinition> definition,
          TextManager&                     textManager);
    /** @brief Destroy scene runtime resources. */
    ~Scene();

    /** @brief Initialize runtime materials and objects for the active
     * definition. */
    bool init();
    /** @brief Update behavior-driven transforms for the current scene time. */
    void update(float sceneElapsedTime);
    /** @brief Render scene geometry and optional text overlay. */
    void render(const Camera& camera, const glm::mat4& projection,
                const glm::mat4& view, float fps, float sceneElapsedTime,
                const UIOverlayConfig& overlayConfig, bool infoOverlayEnabled,
                float currentTimeSeconds);

  private:
    static constexpr std::size_t kBehaviorTypeCount = 4;

    struct PerFrameLightUniforms
    {
        /** Number of active lights written into uniform arrays this frame. */
        int                    lightCount;
        /** World-space light positions padded to max light count. */
        std::vector<glm::vec3> lightPositions;
        /** Final light colors (tint * intensity) padded to max light count. */
        std::vector<glm::vec3> lightColors;
    };

    struct LightUniformNameTable
    {
        /** Last configured max light source capacity. */
        int                      capacity;
        /** Cached uniform names for light position array entries. */
        std::vector<std::string> lightPosNames;
        /** Cached uniform names for light color array entries. */
        std::vector<std::string> lightColorNames;
    };

    using BehaviorHandler = void (Scene::*)(RuntimeSceneObject&, float);

    AssetManager&                    assets;
    TextManager&                     textRenderer;
    std::shared_ptr<SceneDefinition> definition;

    /** Runtime material map keyed by material id. */
    std::unordered_map<std::string, std::shared_ptr<RuntimeMaterial>>
        runtimeMaterials;
    /** Runtime object map keyed by object id. */
    std::unordered_map<std::string, RuntimeSceneObject> runtimeObjects;
    std::vector<RuntimeSceneObject*>                    activeLightSources;
    LightUniformNameTable                               lightUniformNameTable;
    std::array<BehaviorHandler, kBehaviorTypeCount>    behaviorHandlers;

    /** Build shader-backed material instances for the current scene. */
    bool initializeRuntimeMaterials();
    /** Build runtime objects and apply initial transforms from scene data. */
    bool initializeRuntimeObjects();
    /** Refresh cached pointers to objects that act as lights. */
    void refreshActiveLightSources();

    /** Collect per-frame light uniforms from active light sources. */
    PerFrameLightUniforms buildPerFrameLightUniforms(int maxLightSources) const;
    /** Ensure shader light uniform names match current light-array capacity. */
    void ensureLightUniformNameTable(int maxLightSources);
    /** Apply configured behavior for one runtime object. */
    void updateRuntimeObjectBehavior(RuntimeSceneObject& runtimeObject,
                                     float               sceneElapsedTime);
    /** Behavior handlers keyed by BehaviorType. */
    void applyBehaviorNone(RuntimeSceneObject& runtimeObject,
                           float               sceneElapsedTime);
    /** Apply oscillation behavior around initial position. */
    void applyBehaviorOscillate(RuntimeSceneObject& runtimeObject,
                                float               sceneElapsedTime);
    /** Apply spin behavior around configured axis. */
    void applyBehaviorSpin(RuntimeSceneObject& runtimeObject,
                           float               sceneElapsedTime);
    /** Apply linear fly behavior along configured direction. */
    void applyBehaviorFly(RuntimeSceneObject& runtimeObject,
                          float               sceneElapsedTime);
    /** Draw all scene objects with per-shader uniform setup. */
    void renderRuntimeObjects(const Camera& camera, const glm::mat4& projection,
                              const glm::mat4& view, float sceneElapsedTime,
                              int maxLightSources,
                              const PerFrameLightUniforms& perFrameLightUniforms);
    /** Configure shared per-frame uniforms for lit material shaders. */
    void configureLitShaderPerFrame(
        const std::shared_ptr<RuntimeMaterial>& material,
        const Camera& camera, const glm::mat4& projection,
        const glm::mat4& view, float sceneElapsedTime,
        const PerFrameLightUniforms& perFrameLightUniforms) const;
    /** Configure shared per-frame uniforms for light-source shaders. */
    void configureLightSourceShaderPerFrame(
        const std::shared_ptr<RuntimeMaterial>& material,
        const glm::mat4& projection, const glm::mat4& view,
        float sceneElapsedTime) const;
    /** Draw one object with model-space uniforms and indexed geometry. */
    void drawRuntimeObject(const RuntimeSceneObject& runtimeObject) const;

    /** Render static scene text plus optional runtime info overlay. */
    void renderTextOverlay(const UIOverlayConfig& overlayConfig,
                           bool infoOverlayEnabled, float fps,
                           float sceneElapsedTime, float currentTimeSeconds);
};

#endif // SCENE_H

