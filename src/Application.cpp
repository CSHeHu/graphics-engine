#include "Application.h"

#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>

#include "AssetManager.h"
#include "CameraRouteController.h"
#include "Camera.h"
#include "InputManager.h"
#include "Scene.h"
#include "SceneDefinitions.h"
#include "TextManager.h"

Application::Application()
    : window(nullptr, glfwDestroyWindow), camera(nullptr),
    scriptedCameraEnabled(false), activeSceneDefinition(),
    scene(nullptr), assetManager(nullptr), textManager(nullptr),
    inputManager(nullptr), infoOverlayEnabled(false),
    cameraRouteController(nullptr)
{
}

Application::~Application()
{
    scene.reset();
    assetManager.reset();
    textManager.reset();
    inputManager.reset();
    cameraRouteController.reset();
    camera.reset();
    window.reset();
    glfwTerminate();
}

bool Application::init()
{
    const RuntimeConfig& runtimeConfig = SceneDefinitions::getRuntimeConfig();
    const WindowConfig&  windowConfig  = SceneDefinitions::getWindowConfig();

    if (!initWindowAndContext(runtimeConfig, windowConfig))
    {
        return false;
    }

    if (!initSystems(runtimeConfig, windowConfig))
    {
        return false;
    }

    if (!loadInitialScene())
    {
        return false;
    }

    return true;
}

bool Application::initWindowAndContext(const RuntimeConfig& runtimeConfig,
                                       const WindowConfig&  windowConfig)
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,
                   runtimeConfig.opengl.contextVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,
                   runtimeConfig.opengl.contextVersionMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* targetMonitor = nullptr;
    int          targetWidth   = windowConfig.width;
    int          targetHeight  = windowConfig.height;

    if (windowConfig.mode == WindowMode::Fullscreen)
    {
        targetMonitor                = glfwGetPrimaryMonitor();
        const GLFWvidmode* videoMode = glfwGetVideoMode(targetMonitor);
        if (targetMonitor == nullptr || videoMode == nullptr)
        {
            std::cout << "Failed to query primary monitor for fullscreen mode"
                      << std::endl;
            glfwTerminate();
            return false;
        }

        targetWidth  = videoMode->width;
        targetHeight = videoMode->height;
    }

    // glfw window creation
    window.reset(glfwCreateWindow(targetWidth, targetHeight,
                                  runtimeConfig.windowTitle.c_str(),
                                  targetMonitor, NULL));
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window.get());
    return true;
}

bool Application::initSystems(const RuntimeConfig& runtimeConfig,
                              const WindowConfig&  windowConfig)
{
    // Create and set up camera
    camera = std::make_unique<Camera>(
        runtimeConfig.camera.position, glm::vec3(0.0f, 1.0f, 0.0f),
        runtimeConfig.camera.yaw, runtimeConfig.camera.pitch);
    camera->setMovementSpeed(runtimeConfig.camera.speed);
    camera->setMouseSensitivity(runtimeConfig.camera.sensitivity);
    camera->setZoom(runtimeConfig.camera.zoom);
    inputManager = std::make_unique<InputManager>();
    inputManager->registerAsCallbackTarget();
    inputManager->setCamera(camera.get());
    inputManager->setCameraControlEnabled(
        runtimeConfig.input.cameraControlsEnabled);
    cameraRouteController = std::make_unique<CameraRouteController>();

    // callbacks for window resize, mouse movement and scroll movement
    glfwSetFramebufferSizeCallback(window.get(),
                                   InputManager::framebufferSizeCallback);
    glfwSetCursorPosCallback(window.get(), InputManager::mouseCallback);
    glfwSetScrollCallback(window.get(), InputManager::scrollCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window.get(), GLFW_CURSOR,
                     windowConfig.cursorCaptured ? GLFW_CURSOR_DISABLED
                                                 : GLFW_CURSOR_NORMAL);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glfwSwapInterval(windowConfig.vsyncEnabled ? 1 : 0);

    // configure global opengl state
    if (runtimeConfig.rendering.depthTestEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    if (runtimeConfig.rendering.blendEnabled)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    // Scene order comes from content config instead of app lifecycle code.
    if (!scenePlaylist.initialize(SceneDefinitions::getDefaultSceneCycle()))
    {
        std::cout << "No scenes configured for scene cycle" << std::endl;
        return false;
    }

    assetManager = std::make_unique<AssetManager>();

    // Initialize text manager with config
    const UIOverlayConfig& uiConfig = SceneDefinitions::getUIOverlayConfig();
    textManager                     = std::make_unique<TextManager>();
    if (!textManager->init(uiConfig.fontPath, uiConfig.vertexShaderPath,
                           uiConfig.fragmentShaderPath))
    {
        std::cout << "Failed to initialize text manager" << std::endl;
        return false;
    }
    infoOverlayEnabled = uiConfig.enabled;

    return true;
}

bool Application::loadInitialScene()
{
    timeState.initialize(static_cast<float>(glfwGetTime()));
    if (!loadSceneById(scenePlaylist.activeSceneId))
    {
        std::cout << "Failed to initialize scene" << std::endl;
        return false;
    }

    return true;
}

void Application::run()
{
    constexpr float TIME_STEP_SECONDS = 0.25f;

    // render loop
    while (!glfwWindowShouldClose(window.get()))
    {
        const float nowRealSeconds = static_cast<float>(glfwGetTime());
        const float realDeltaSeconds =
            timeState.computeRealDelta(nowRealSeconds);

        inputManager->processInput(window.get(), realDeltaSeconds);
        const InputActions actions = inputManager->consumeActions();

        if (actions.toggleCameraMode)
        {
            toggleCameraMode();
        }

        if (actions.toggleInfoOverlay)
        {
            infoOverlayEnabled = !infoOverlayEnabled;
        }

        if (actions.togglePause)
        {
            timeState.paused = !timeState.paused;
        }

        if (actions.stepTimeForward)
        {
            timeState.stepForward(TIME_STEP_SECONDS);
        }

        if (actions.stepTimeBackward)
        {
            timeState.stepBackward(TIME_STEP_SECONDS);
        }

        timeState.advance(realDeltaSeconds);

        const SceneTimelinePosition timelinePosition =
            scenePlaylist.resolve(timeState.currentTimeSeconds);
        if (scenePlaylist.needsSwitch(timelinePosition))
        {
            const SceneId targetSceneId =
                scenePlaylist.sceneIdAt(timelinePosition.index);
            if (loadSceneById(targetSceneId))
            {
                scenePlaylist.commit(timelinePosition);
            }
            else
            {
                std::cout << "Failed to switch scene" << std::endl;
            }
        }
        else
        {
            scenePlaylist.commit(timelinePosition);
        }

        if (scriptedCameraEnabled)
        {
            const float sceneElapsed =
                timeState.currentTimeSeconds -
                scenePlaylist.activeSceneStartTimeSeconds;
            cameraRouteController->apply(*camera, activeSceneDefinition->camera,
                                         sceneElapsed);
        }

        // render
        renderFrame();

        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window.get());
        glfwPollEvents();
    }
}

void Application::renderFrame()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int framebufferWidth  = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window.get(), &framebufferWidth, &framebufferHeight);

    const RuntimeConfig& runtimeConfig = SceneDefinitions::getRuntimeConfig();
    const float aspectRatio = (framebufferHeight > 0)
                                  ? static_cast<float>(framebufferWidth) /
                                        static_cast<float>(framebufferHeight)
                                  : 1.0f;

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(camera->getZoom()), aspectRatio,
                                  runtimeConfig.rendering.nearPlane,
                                  runtimeConfig.rendering.farPlane);

    glm::mat4 view = glm::mat4(1.0f);
    view           = camera->GetViewMatrix();

    float fps = (timeState.deltaTimeSeconds > 0.0f)
                    ? 1.0f / timeState.deltaTimeSeconds
                    : 0.0f;
    float sceneElapsedTime =
        timeState.currentTimeSeconds - scenePlaylist.activeSceneStartTimeSeconds;
    const UIOverlayConfig& overlayConfig =
        SceneDefinitions::getUIOverlayConfig();

    scene->update(sceneElapsedTime);
    scene->render(*camera, projection, view, fps, sceneElapsedTime,
                  overlayConfig, infoOverlayEnabled,
                  timeState.currentTimeSeconds);
}

bool Application::loadSceneById(SceneId id)
{
    if (!assetManager)
    {
        return false;
    }

    std::shared_ptr<SceneDefinition> definition;
    if (!SceneDefinitions::tryCreateSceneDefinition(id, definition))
    {
        return false;
    }

    activeSceneDefinition = definition;
    scriptedCameraEnabled =
        activeSceneDefinition->camera.mode == CameraMode::Scripted &&
        !activeSceneDefinition->camera.keyframes.empty();
    refreshCameraControlMode();
    if (scriptedCameraEnabled &&
        !activeSceneDefinition->camera.keyframes.empty())
    {
        const CameraKeyframe& startKeyframe =
            activeSceneDefinition->camera.keyframes.front();
        camera->SetPoseLookAt(startKeyframe.position, startKeyframe.lookAt);
    }

    std::unique_ptr<Scene> nextScene =
        std::make_unique<Scene>(*assetManager, definition, *textManager);
    if (!nextScene->init())
    {
        return false;
    }

    scene = std::move(nextScene);
    return true;
}

void Application::refreshCameraControlMode()
{
    inputManager->setCameraControlEnabled(!scriptedCameraEnabled);
}

void Application::toggleCameraMode()
{
    if (!activeSceneDefinition->camera.keyframes.empty())
    {
        scriptedCameraEnabled = !scriptedCameraEnabled;
        refreshCameraControlMode();
    }
}
