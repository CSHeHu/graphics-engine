#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstddef>
#include <glm.hpp>
#include <memory>

#include "SceneDefinition.h"
#include "SceneDefinitions.h"
#include "ScenePlaylist.h"
#include "TimeState.h"

class Camera;
class Scene;
class AssetManager;
class TextManager;
class InputManager;
class CameraRouteController;

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

    TimeState     timeState;
    ScenePlaylist scenePlaylist;

    std::unique_ptr<Scene>                 scene;
    std::unique_ptr<AssetManager>          assetManager;
    std::unique_ptr<TextManager>           textManager;
    std::unique_ptr<InputManager>          inputManager;
    bool                                   infoOverlayEnabled;
    std::unique_ptr<CameraRouteController> cameraRouteController;

    /** @brief Load and activate scene runtime objects for a scene id. */
    bool loadSceneById(SceneId id);
    /** @brief Initialize GLFW window and OpenGL context. */
    bool initWindowAndContext(const RuntimeConfig& runtimeConfig,
                              const WindowConfig&  windowConfig);
    /** @brief Initialize camera, input, render state, and shared managers. */
    bool initSystems(const RuntimeConfig& runtimeConfig,
                     const WindowConfig&  windowConfig);
    /** @brief Initialize simulation time and load the initial scene. */
    bool loadInitialScene();
    /** @brief Render one frame for the active scene. */
    void renderFrame();
    /** @brief Enable or disable manual camera controls based on current mode.
     */
    void refreshCameraControlMode();
    /** @brief Toggle between scripted and manual camera modes when available.
     */
    void toggleCameraMode();
};

#endif // APPLICATION_H
