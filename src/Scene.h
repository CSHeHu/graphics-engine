#ifndef SCENE_H
#define SCENE_H

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
class ShadowManager;

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
    std::size_t                      vertexCount;
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
    Scene(AssetManager& assetManager, SceneDefinition definition,
          TextManager* textManager);
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
    AssetManager&   assets;
    TextManager*    textRenderer;
    SceneDefinition definition;

    /** Runtime material map keyed by material id. */
    std::unordered_map<std::string, std::shared_ptr<RuntimeMaterial>>
        runtimeMaterials;
    /** Runtime object map keyed by object id. */
    std::unordered_map<std::string, RuntimeSceneObject> runtimeObjects;
    std::vector<RuntimeSceneObject*>                    activeLightSources;

    std::unique_ptr<ShadowManager> shadowManager;

    /** Build shader-backed material instances for the current scene. */
    bool initializeRuntimeMaterials();
    /** Build runtime objects and apply initial transforms from scene data. */
    bool initializeRuntimeObjects();
    /** Refresh cached pointers to objects that act as lights. */
    void refreshActiveLightSources();
    /** Initialize or disable shadow resources based on scene settings. */
    bool configureShadowManager();

    /** Collect per-frame light uniform arrays from active light sources. */
    void computeLightUniformData(int maxLightSources, int& lightCount,
                                 std::vector<glm::vec3>& lightPositions,
                                 std::vector<glm::vec3>& lightColors) const;
    /** Submit depth-only geometry before the main color pass. */
    void renderShadowDepthPass(const std::vector<glm::vec3>& lightPositions,
                               unsigned int shadowUpdateIntervalFrames);
    /** Bind the shadow texture if shadows are active and ready. */
    void bindShadowTextureIfEnabled(int shadowMapTextureUnit) const;
    /** Draw all scene objects with per-shader uniform setup. */
    void renderRuntimeObjects(const Camera& camera, const glm::mat4& projection,
                              const glm::mat4& view, float sceneElapsedTime,
                              int maxLightSources, int lightCount,
                              const std::vector<glm::vec3>& lightPositions,
                              const std::vector<glm::vec3>& lightColors,
                              int shadowMapTextureUnit);

    void renderTextOverlay(const UIOverlayConfig& overlayConfig,
                           bool infoOverlayEnabled, float fps,
                           float sceneElapsedTime, float currentTimeSeconds);
};

#endif // SCENE_H
