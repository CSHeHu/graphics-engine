#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstddef>
#include <glm.hpp>
#include <memory>
#include <vector>

#include "SceneDefinitions.h"
#include "SceneDefinition.h"

class Camera;
class Scene;
class AssetManager;

class Application
{
public:
    Application();
    ~Application();

    bool init();
    void run();

private:
    using WindowHandle = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)>;

    // Window and GL context
    WindowHandle window;
    static const unsigned int SCR_WIDTH = 1920;
    static const unsigned int SCR_HEIGHT = 1080;

    // Camera
    std::unique_ptr<Camera> camera;
    bool scriptedCameraEnabled;
    bool cameraModeToggleLatch;
    SceneDefinition activeSceneDefinition;

    // Timing
    float currentFrame;
    float deltaTime;
    float lastFrame;
    float lastSceneSwitchTime;
    SceneId activeSceneId;
    // Scene cycle comes from assets/scenes/scene_config.json.
    // Switching stops at the end of the configured cycle.
    std::vector<SceneCycleEntry> sceneCycle;
    std::size_t sceneCyclePosition;

    // Scene
    std::unique_ptr<Scene> scene;
    std::unique_ptr<AssetManager> assetManager;

    // Helper
    bool loadSceneById(SceneId id);
    void renderFrame();
    void applyScriptedCamera(float sceneElapsedTimeSeconds);
    void refreshCameraControlMode();
    static glm::vec3 lerpVec3(const glm::vec3 &a, const glm::vec3 &b, float t);
};

#endif // APPLICATION_H
