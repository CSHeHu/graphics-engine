#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <memory>

class Camera;
class Scene;

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

    // Scene
    std::unique_ptr<Scene> scene;

    // Helper
    void renderFrame();
};

#endif // APPLICATION_H
