#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstddef>
#include <glm.hpp>
#include <memory>
#include <vector>

#include "SceneDefinition.h"
#include "SceneDefinitions.h"

class Camera;
class Scene;
class AssetManager;
class TextManager;

/**
 * @brief Main application controller for initialization, update loop and rendering.
 */
class Application
{
public:
    /** @brief Construct application state. */
    Application();
    /** @brief Release runtime resources and terminate GLFW. */
    ~Application();

    /** @brief Initialize window, graphics context, systems and first scene. */
    bool init();
    /** @brief Run the main loop until window close is requested. */
    void run();

private:
    using WindowHandle = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)>;

    WindowHandle window;

    std::unique_ptr<Camera> camera;
    bool scriptedCameraEnabled;
    SceneDefinition activeSceneDefinition;

    float currentTimeSeconds;
    float deltaTime;
    float lastRealTimeSeconds;
    float lastSceneSwitchTime;
    SceneId activeSceneId;
    /** Ordered scene playback plan loaded from scene configuration. */
    std::vector<SceneCycleEntry> sceneCycle;
    std::size_t sceneCyclePosition;

    std::unique_ptr<Scene> scene;
    std::unique_ptr<AssetManager> assetManager;
    std::unique_ptr<TextManager> textManager;
    bool infoOverlayEnabled;
    bool paused;

    /** @brief Load and activate scene runtime objects for a scene id. */
    bool loadSceneById(SceneId id);
    /** @brief Render one frame for the active scene. */
    void renderFrame();
    /** @brief Apply scripted camera route at scene-local elapsed time. */
    void applyScriptedCamera(float sceneElapsedTimeSeconds);
    /** @brief Enable or disable manual camera controls based on current mode. */
    void refreshCameraControlMode();
    /** @brief Toggle between scripted and manual camera modes when available. */
    void toggleCameraMode();
    /** @brief Linear interpolation helper for camera path blending. */
    static glm::vec3 lerpVec3(const glm::vec3 &a, const glm::vec3 &b, float t);
};

#endif // APPLICATION_H
