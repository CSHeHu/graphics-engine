#ifndef SCENE_H
#define SCENE_H

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <glm.hpp>

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
    RenderMode renderMode;
    glm::vec3 objectColor;
};

/** @brief Runtime object state used for rendering and behavior updates. */
struct RuntimeSceneObject
{
    std::shared_ptr<Object> object;
    std::shared_ptr<RuntimeMaterial> material;
    std::size_t vertexCount;
    SceneRole role;
    BehaviorType behavior;
    float behaviorSpeed;
    glm::vec3 behaviorAxis;
    float behaviorAmplitude;
    glm::vec3 initialPosition;
    float initialRotationAngle;
};

/**
 * @brief Runtime scene container that updates behaviors and renders scene content.
 */
class Scene
{
public:
    /** @brief Construct scene runtime from parsed definition and shared managers. */
    Scene(AssetManager &assetManager, SceneDefinition definition, TextManager *textManager);
    /** @brief Destroy scene runtime resources. */
    ~Scene();

    /** @brief Initialize runtime materials and objects for the active definition. */
    bool init();
    /** @brief Update behavior-driven transforms for the current frame. */
    void update(float deltaTime, float sceneElapsedTime);
    /** @brief Render scene geometry and optional text overlay. */
    void render(const Camera &camera, const glm::mat4 &projection, const glm::mat4 &view, float fps, float sceneElapsedTime, const UIOverlayConfig &overlayConfig, bool infoOverlayEnabled);

private:
    AssetManager &assets;
    TextManager *textRenderer;
    SceneDefinition definition;

    /** Runtime material map keyed by material id. */
    std::unordered_map<std::string, std::shared_ptr<RuntimeMaterial>> runtimeMaterials;
    /** Runtime object map keyed by object id. */
    std::unordered_map<std::string, RuntimeSceneObject> runtimeObjects;
    std::shared_ptr<Object> activeLightSource;
};

#endif // SCENE_H
