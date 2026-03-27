#ifndef APPLICATION_H
#define APPLICATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <memory>
#include <cmath>
#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "InputManager.h"
#include "MeshData.h"
#include "Object.h"

class Application
{
public:
    Application();
    ~Application();

    bool init();
    void run();

private:
    // Window and GL context
    GLFWwindow *window;
    static const unsigned int SCR_WIDTH = 1920;
    static const unsigned int SCR_HEIGHT = 1080;

    // Camera
    Camera *camera;

    // Timing
    float deltaTime;
    float lastFrame;

    // Scene objects
    std::shared_ptr<Object> lightCube;
    std::shared_ptr<Object> lightTargetCube;
    std::shared_ptr<Object> ground;

    // Shaders
    std::shared_ptr<Shader> lightShader;
    std::shared_ptr<Shader> lightTargetShader;

    // Helper
    void renderFrame();
};

#endif // APPLICATION_H
