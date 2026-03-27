#include "Application.h"

#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>

#include "AssetManager.h"
#include "Camera.h"
#include "InputManager.h"
#include "Scene.h"
#include "SceneDefinitions.h"

Application::Application()
    : window(nullptr, glfwDestroyWindow),
      camera(nullptr),
      deltaTime(0.0f),
      lastFrame(0.0f),
      lastSceneSwitchTime(0.0f),
      activeSceneIndex(0),
      sceneCycleIndices(),
      sceneCyclePosition(0),
      scene(nullptr),
      assetManager(nullptr),
      currentFrame(0.0f)
{
}

Application::~Application()
{
    scene.reset();
    assetManager.reset();
    camera.reset();
    window.reset();
    glfwTerminate();
}

bool Application::init()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    window.reset(glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL));
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window.get());

    // Create and set up camera
    camera = std::make_unique<Camera>(glm::vec3(0.0f, 5.0f, 3.0f));
    InputManager::setCamera(camera.get());

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

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Define scene playback order here. Add more ids to extend the cycle.
    sceneCycleIndices = {0, 1, 1, 0};
    sceneCyclePosition = 0;

    assetManager = std::make_unique<AssetManager>();
    if (sceneCycleIndices.empty())
    {
        std::cout << "No scenes configured for scene cycle" << std::endl;
        return false;
    }

    activeSceneIndex = sceneCycleIndices[sceneCyclePosition];
    lastSceneSwitchTime = 0.0f;
    if (!loadSceneByIndex(activeSceneIndex))
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

        // Switch to next configured scene index; stop switching when cycle ends.
        if (currentFrame - lastSceneSwitchTime >= 5.0f)
        {
            if (sceneCyclePosition + 1 < sceneCycleIndices.size())
            {
                const std::size_t nextCyclePosition = sceneCyclePosition + 1;
                const int nextSceneIndex = sceneCycleIndices[nextCyclePosition];
                if (loadSceneByIndex(nextSceneIndex))
                {
                    sceneCyclePosition = nextCyclePosition;
                    activeSceneIndex = nextSceneIndex;
                    lastSceneSwitchTime = currentFrame;
                }
                else
                {
                    std::cout << "Failed to switch scene" << std::endl;
                }
            }
        }

        // input
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

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                  0.1f, 100.0f);

    glm::mat4 view = glm::mat4(1.0f);
    view = camera->GetViewMatrix();

    scene->update(deltaTime);
    scene->render(*camera, projection, view);
}

bool Application::loadSceneByIndex(int index)
{
    if (!assetManager)
    {
        return false;
    }

    SceneDefinition definition;
    if (index == 0)
    {
        // id 0 -> basic scene
        definition = createBasicSceneDefinition();
    }
    else if (index == 1)
    {
        // id 1 -> alternate scene
        definition = createAlternateSceneDefinition();
    }
    else
    {
        return false;
    }

    std::unique_ptr<Scene> nextScene = std::make_unique<Scene>(*assetManager, definition);
    if (!nextScene->init())
    {
        return false;
    }

    scene = std::move(nextScene);
    return true;
}