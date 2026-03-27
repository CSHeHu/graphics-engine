#include "Application.h"

#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>

#include "Camera.h"
#include "InputManager.h"
#include "Scene.h"

Application::Application()
    : window(nullptr, glfwDestroyWindow), camera(nullptr), deltaTime(0.0f), lastFrame(0.0f), scene(nullptr)
{
}

Application::~Application()
{
    scene.reset();
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

    scene = std::make_unique<Scene>();
    if (!scene->init())
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
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

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

    scene->update(static_cast<float>(glfwGetTime()));
    scene->render(*camera, projection, view);
}