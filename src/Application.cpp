#include "Application.h"

#include <cmath>
#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>

#include "AssetManager.h"
#include "Camera.h"
#include "InputManager.h"
#include "Scene.h"
#include "SceneDefinitions.h"
#include "TextManager.h"

Application::Application()
    : window(nullptr, glfwDestroyWindow), camera(nullptr),
    scriptedCameraEnabled(false), activeSceneDefinition(),
    scene(nullptr), assetManager(nullptr), textManager(nullptr),
    infoOverlayEnabled(false)
{
}

Application::~Application()
{
    scene.reset();
    assetManager.reset();
    textManager.reset();
    camera.reset();
    window.reset();
    glfwTerminate();
}

bool Application::init()
{
    const RuntimeConfig& runtimeConfig = SceneDefinitions::getRuntimeConfig();

    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,
                   runtimeConfig.opengl.contextVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,
                   runtimeConfig.opengl.contextVersionMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const WindowConfig& windowConfig = SceneDefinitions::getWindowConfig();

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

    // Create and set up camera
    camera = std::make_unique<Camera>(
        runtimeConfig.camera.position, glm::vec3(0.0f, 1.0f, 0.0f),
        runtimeConfig.camera.yaw, runtimeConfig.camera.pitch);
    camera->setMovementSpeed(runtimeConfig.camera.speed);
    camera->setMouseSensitivity(runtimeConfig.camera.sensitivity);
    camera->setZoom(runtimeConfig.camera.zoom);
    InputManager::setCamera(camera.get());
    InputManager::setCameraControlEnabled(
        runtimeConfig.input.cameraControlsEnabled);

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

        InputManager::processInput(window.get(), realDeltaSeconds);
        const InputActions actions = InputManager::consumeActions();

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
            applyScriptedCamera(sceneElapsed);
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

void Application::applyScriptedCamera(float sceneElapsedTimeSeconds)
{
    const std::vector<CameraKeyframe>& keyframes =
        activeSceneDefinition->camera.keyframes;
    if (keyframes.empty())
    {
        return;
    }

    if (keyframes.size() == 1)
    {
        camera->SetPoseLookAt(keyframes.front().position,
                              keyframes.front().lookAt);
        return;
    }

    const float routeEnd = keyframes.back().timeSeconds;
    if (routeEnd <= 0.0f)
    {
        camera->SetPoseLookAt(keyframes.back().position,
                              keyframes.back().lookAt);
        return;
    }

    float routeTime = sceneElapsedTimeSeconds;
    if (activeSceneDefinition->camera.loop)
    {
        routeTime = std::fmod(routeTime, routeEnd);
    }
    else if (routeTime >= routeEnd)
    {
        camera->SetPoseLookAt(keyframes.back().position,
                              keyframes.back().lookAt);
        return;
    }

    for (std::size_t i = 0; i + 1 < keyframes.size(); ++i)
    {
        const CameraKeyframe& a = keyframes[i];
        const CameraKeyframe& b = keyframes[i + 1];
        if (routeTime >= a.timeSeconds && routeTime <= b.timeSeconds)
        {
            const float segmentDuration = b.timeSeconds - a.timeSeconds;
            const float t = segmentDuration > 0.0f
                                ? (routeTime - a.timeSeconds) / segmentDuration
                                : 0.0f;
            const glm::vec3 position = lerpVec3(a.position, b.position, t);
            const glm::vec3 lookAt   = lerpVec3(a.lookAt, b.lookAt, t);
            camera->SetPoseLookAt(position, lookAt);
            return;
        }
    }

    camera->SetPoseLookAt(keyframes.back().position, keyframes.back().lookAt);
}

void Application::refreshCameraControlMode()
{
    InputManager::setCameraControlEnabled(!scriptedCameraEnabled);
}

void Application::toggleCameraMode()
{
    if (!activeSceneDefinition->camera.keyframes.empty())
    {
        scriptedCameraEnabled = !scriptedCameraEnabled;
        refreshCameraControlMode();
    }
}

glm::vec3 Application::lerpVec3(const glm::vec3& a, const glm::vec3& b, float t)
{
    return a + (b - a) * t;
}
