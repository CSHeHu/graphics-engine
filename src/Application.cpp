#include "Application.h"

#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>

#include "AssetManager.h"
#include "Camera.h"
#include "InputManager.h"
#include "Scene.h"
#include "SceneDefinitions.h"
#include "TextManager.h"

Application::Application()
    : window(nullptr, glfwDestroyWindow),
      camera(nullptr),
      scriptedCameraEnabled(false),
      activeSceneDefinition(),
      deltaTime(0.0f),
      lastFrame(0.0f),
      lastSceneSwitchTime(0.0f),
      activeSceneId(SceneId::Basic),
      sceneCycle(),
      sceneCyclePosition(0),
      scene(nullptr),
      assetManager(nullptr),
      currentFrame(0.0f),
      textManager(nullptr),
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
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_CONTEXT_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_CONTEXT_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const WindowConfig &windowConfig = SceneDefinitions::getWindowConfig();

    GLFWmonitor *targetMonitor = nullptr;
    int targetWidth = windowConfig.width;
    int targetHeight = windowConfig.height;

    if (windowConfig.mode == WindowMode::Fullscreen)
    {
        targetMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *videoMode = glfwGetVideoMode(targetMonitor);
        if (targetMonitor == nullptr || videoMode == nullptr)
        {
            std::cout << "Failed to query primary monitor for fullscreen mode" << std::endl;
            glfwTerminate();
            return false;
        }

        targetWidth = videoMode->width;
        targetHeight = videoMode->height;
    }

    // glfw window creation
    window.reset(glfwCreateWindow(targetWidth, targetHeight, WINDOW_TITLE, targetMonitor, NULL));
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window.get());

    // Create and set up camera
    camera = std::make_unique<Camera>(glm::vec3(CAMERA_DEFAULT_X, CAMERA_DEFAULT_Y, CAMERA_DEFAULT_Z));
    InputManager::setCamera(camera.get());
    InputManager::setCameraControlEnabled(true);
    InputManager::setCameraModeToggleCallback([this]()
                                              { this->toggleCameraMode(); });
    InputManager::setInfoOverlayToggleCallback([this]()
                                               { this->infoOverlayEnabled = !this->infoOverlayEnabled; });

    // callbacks for window resize, mouse movement and scroll movement
    glfwSetFramebufferSizeCallback(window.get(), InputManager::framebufferSizeCallback);
    glfwSetCursorPosCallback(window.get(), InputManager::mouseCallback);
    glfwSetScrollCallback(window.get(), InputManager::scrollCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Disable VSync to test uncapped FPS
    glfwSwapInterval(0);

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Scene order comes from content config instead of app lifecycle code.
    sceneCycle = SceneDefinitions::getDefaultSceneCycle();
    sceneCyclePosition = 0;

    assetManager = std::make_unique<AssetManager>();
    if (sceneCycle.empty())
    {
        std::cout << "No scenes configured for scene cycle" << std::endl;
        return false;
    }

    // Initialize text manager with config
    const UIOverlayConfig &uiConfig = SceneDefinitions::getUIOverlayConfig();
    textManager = std::make_unique<TextManager>();
    if (!textManager->init(uiConfig.fontPath, uiConfig.vertexShaderPath, uiConfig.fragmentShaderPath))
    {
        std::cout << "Failed to initialize text manager" << std::endl;
        return false;
    }

    activeSceneId = sceneCycle[sceneCyclePosition].id;
    lastSceneSwitchTime = 0.0f;
    if (!loadSceneById(activeSceneId))
    {
        std::cout << "Failed to initialize scene" << std::endl;
        return false;
    }

    return true;
}

void Application::run()
{
    // render loop
    while (!glfwWindowShouldClose(window.get()))
    {
        // per-frame time logic
        currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        const float sceneDurationSeconds = sceneCycle[sceneCyclePosition].durationSeconds;

        // Move forward in the configured scene sequence using per-entry duration.
        if (sceneDurationSeconds > 0.0f && currentFrame - lastSceneSwitchTime >= sceneDurationSeconds)
        {
            if (sceneCyclePosition + 1 < sceneCycle.size())
            {
                const std::size_t nextCyclePosition = sceneCyclePosition + 1;
                const SceneId nextSceneId = sceneCycle[nextCyclePosition].id;
                if (loadSceneById(nextSceneId))
                {
                    sceneCyclePosition = nextCyclePosition;
                    activeSceneId = nextSceneId;
                    lastSceneSwitchTime = currentFrame;
                }
                else
                {
                    std::cout << "Failed to switch scene" << std::endl;
                }
            }
        }

        if (scriptedCameraEnabled)
        {
            const float sceneElapsed = currentFrame - lastSceneSwitchTime;
            applyScriptedCamera(sceneElapsed);
        }

        InputManager::processInput(window.get(), deltaTime);

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

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window.get(), &framebufferWidth, &framebufferHeight);

    const float aspectRatio = (framebufferHeight > 0)
                                  ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight)
                                  : static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(camera->Zoom), aspectRatio, NEAR_PLANE, FAR_PLANE);

    glm::mat4 view = glm::mat4(1.0f);
    view = camera->GetViewMatrix();

    float fps = (deltaTime > 0.0f) ? 1.0f / deltaTime : 0.0f;
    float sceneElapsedTime = currentFrame - lastSceneSwitchTime;
    const UIOverlayConfig &overlayConfig = SceneDefinitions::getUIOverlayConfig();

    scene->update(deltaTime);
    scene->render(*camera, projection, view, fps, sceneElapsedTime, overlayConfig, infoOverlayEnabled);
}

bool Application::loadSceneById(SceneId id)
{
    if (!assetManager)
    {
        return false;
    }

    SceneDefinition definition;
    if (!SceneDefinitions::tryCreateSceneDefinition(id, definition))
    {
        return false;
    }

    activeSceneDefinition = definition;
    scriptedCameraEnabled = activeSceneDefinition.camera.mode == CameraMode::Scripted && !activeSceneDefinition.camera.keyframes.empty();
    refreshCameraControlMode();
    if (scriptedCameraEnabled && !activeSceneDefinition.camera.keyframes.empty())
    {
        const CameraKeyframe &startKeyframe = activeSceneDefinition.camera.keyframes.front();
        camera->SetPoseLookAt(startKeyframe.position, startKeyframe.lookAt);
    }

    std::unique_ptr<Scene> nextScene = std::make_unique<Scene>(*assetManager, definition, textManager.get());
    if (!nextScene->init())
    {
        return false;
    }

    scene = std::move(nextScene);
    return true;
}

void Application::applyScriptedCamera(float sceneElapsedTimeSeconds)
{
    const std::vector<CameraKeyframe> &keyframes = activeSceneDefinition.camera.keyframes;
    if (keyframes.empty())
    {
        return;
    }

    if (keyframes.size() == 1)
    {
        camera->SetPoseLookAt(keyframes.front().position, keyframes.front().lookAt);
        return;
    }

    const float routeEnd = keyframes.back().timeSeconds;
    if (routeEnd <= 0.0f)
    {
        camera->SetPoseLookAt(keyframes.back().position, keyframes.back().lookAt);
        return;
    }

    float routeTime = sceneElapsedTimeSeconds;
    if (activeSceneDefinition.camera.loop)
    {
        routeTime = std::fmod(routeTime, routeEnd);
    }
    else if (routeTime >= routeEnd)
    {
        camera->SetPoseLookAt(keyframes.back().position, keyframes.back().lookAt);
        return;
    }

    for (std::size_t i = 0; i + 1 < keyframes.size(); ++i)
    {
        const CameraKeyframe &a = keyframes[i];
        const CameraKeyframe &b = keyframes[i + 1];
        if (routeTime >= a.timeSeconds && routeTime <= b.timeSeconds)
        {
            const float segmentDuration = b.timeSeconds - a.timeSeconds;
            const float t = segmentDuration > 0.0f ? (routeTime - a.timeSeconds) / segmentDuration : 0.0f;
            const glm::vec3 position = lerpVec3(a.position, b.position, t);
            const glm::vec3 lookAt = lerpVec3(a.lookAt, b.lookAt, t);
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
    if (!activeSceneDefinition.camera.keyframes.empty())
    {
        scriptedCameraEnabled = !scriptedCameraEnabled;
        refreshCameraControlMode();
    }
}

glm::vec3 Application::lerpVec3(const glm::vec3 &a, const glm::vec3 &b, float t)
{
    return a + (b - a) * t;
}