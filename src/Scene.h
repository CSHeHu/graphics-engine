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

struct RuntimeMaterial
{
    std::shared_ptr<Shader> shader;
    RenderMode renderMode;
    glm::vec3 objectColor;
};

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
};

class Scene
{
public:
    Scene(AssetManager &assetManager, SceneDefinition definition, TextManager *textManager);
    ~Scene();

    bool init();
    void update(float deltaTime, float sceneElapsedTime);
    void render(const Camera &camera, const glm::mat4 &projection, const glm::mat4 &view, float fps, float sceneElapsedTime, const UIOverlayConfig &overlayConfig, bool infoOverlayEnabled);

private:
    AssetManager &assets;
    TextManager *textRenderer;
    SceneDefinition definition;

    std::unordered_map<std::string, std::shared_ptr<RuntimeMaterial>> runtimeMaterials;
    std::unordered_map<std::string, RuntimeSceneObject> runtimeObjects;
    std::shared_ptr<Object> activeLightSource;
};

#endif // SCENE_H
