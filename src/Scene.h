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

struct RuntimeSceneObject
{
    std::shared_ptr<Object> object;
    std::size_t vertexCount;
    std::string renderMode;
    glm::vec3 objectColor;
};

class Scene
{
public:
    Scene(AssetManager &assetManager, SceneDefinition definition);
    ~Scene();

    bool init();
    void update(float deltaTime);
    void render(const Camera &camera, const glm::mat4 &projection, const glm::mat4 &view);

private:
    AssetManager &assets;
    SceneDefinition definition;
    float elapsedTime;

    std::unordered_map<std::string, RuntimeSceneObject> runtimeObjects;

    std::shared_ptr<Object> lightCube;
    std::shared_ptr<Object> lightTargetCube;
    std::shared_ptr<Object> ground;
};

#endif // SCENE_H
