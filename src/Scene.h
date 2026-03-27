#ifndef SCENE_H
#define SCENE_H

#include <cstddef>
#include <memory>
#include <glm.hpp>

class Camera;
class Object;
class Shader;
class AssetManager;

class Scene
{
public:
    explicit Scene(AssetManager &assetManager);
    ~Scene();

    bool init();
    void update(float deltaTime);
    void render(const Camera &camera, const glm::mat4 &projection, const glm::mat4 &view);

private:
    AssetManager &assets;
    float elapsedTime;
    std::size_t cubeVertexCount;
    std::size_t groundVertexCount;

    std::shared_ptr<Object> lightCube;
    std::shared_ptr<Object> lightTargetCube;
    std::shared_ptr<Object> ground;

    std::shared_ptr<Shader> lightShader;
    std::shared_ptr<Shader> lightTargetShader;
};

#endif // SCENE_H
