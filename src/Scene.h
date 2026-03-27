#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <glm.hpp>

class Camera;
class Object;
class Shader;

class Scene
{
public:
    Scene();
    ~Scene();

    bool init();
    void update(float timeSeconds);
    void render(const Camera &camera, const glm::mat4 &projection, const glm::mat4 &view);

private:
    float elapsedTime;

    std::shared_ptr<Object> lightCube;
    std::shared_ptr<Object> lightTargetCube;
    std::shared_ptr<Object> ground;

    std::shared_ptr<Shader> lightShader;
    std::shared_ptr<Shader> lightTargetShader;
};

#endif // SCENE_H
