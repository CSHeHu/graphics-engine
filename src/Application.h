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

struct TimeState
{
  float currentTimeSeconds;
  float deltaTimeSeconds;
  float lastRealTimeSeconds;
  bool  paused;

  void initialize(float nowRealTimeSeconds)
  {
    currentTimeSeconds  = 0.0f;
    deltaTimeSeconds    = 0.0f;
    lastRealTimeSeconds = nowRealTimeSeconds;
    paused              = false;
  }

  float computeRealDelta(float nowRealTimeSeconds)
  {
    float realDeltaSeconds = nowRealTimeSeconds - lastRealTimeSeconds;
    if (realDeltaSeconds < 0.0f)
    {
      realDeltaSeconds = 0.0f;
    }
    lastRealTimeSeconds = nowRealTimeSeconds;
    return realDeltaSeconds;
  }

  void stepForward(float stepSeconds)
  {
    currentTimeSeconds += stepSeconds;
  }

  void stepBackward(float stepSeconds)
  {
    currentTimeSeconds -= stepSeconds;
    if (currentTimeSeconds < 0.0f)
    {
      currentTimeSeconds = 0.0f;
    }
  }

  void advance(float realDeltaSeconds)
  {
    if (!paused)
    {
      currentTimeSeconds += realDeltaSeconds;
      deltaTimeSeconds = realDeltaSeconds;
      return;
    }

    deltaTimeSeconds = 0.0f;
  }
};

/**
 * @brief Main application controller for initialization, update loop and
 * rendering.
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
    using WindowHandle = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow*)>;

    WindowHandle window;

    std::unique_ptr<Camera>          camera;
    bool                             scriptedCameraEnabled;
    std::shared_ptr<SceneDefinition> activeSceneDefinition;

    TimeState timeState;
    float   lastSceneSwitchTime;
    SceneId activeSceneId;
    /** Ordered scene playback plan loaded from scene configuration. */
    std::vector<SceneCycleEntry> sceneCycle;
    std::size_t                  sceneCyclePosition;

    std::unique_ptr<Scene>        scene;
    std::unique_ptr<AssetManager> assetManager;
    std::unique_ptr<TextManager>  textManager;
    bool                          infoOverlayEnabled;

    /** @brief Load and activate scene runtime objects for a scene id. */
    bool loadSceneById(SceneId id);
    /** @brief Render one frame for the active scene. */
    void renderFrame();
    /** @brief Apply scripted camera route at scene-local elapsed time. */
    void applyScriptedCamera(float sceneElapsedTimeSeconds);
    /** @brief Enable or disable manual camera controls based on current mode.
     */
    void refreshCameraControlMode();
    /** @brief Toggle between scripted and manual camera modes when available.
     */
    void toggleCameraMode();
    /** @brief Linear interpolation helper for camera path blending. */
    static glm::vec3 lerpVec3(const glm::vec3& a, const glm::vec3& b, float t);
};

#endif // APPLICATION_H
