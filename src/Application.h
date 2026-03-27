#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <memory>

class Camera;
class Scene;
class AssetManager;

class Application
{
public:
    Application();
    ~Application();

    bool init();
    void run();

private:
    using WindowHandle = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)>;

    // Window and GL context
    WindowHandle window;
    static const unsigned int SCR_WIDTH = 1920;
    static const unsigned int SCR_HEIGHT = 1080;

    // Camera
    std::unique_ptr<Camera> camera;

    // Timing
    float currentFrame;
    float deltaTime;
    float lastFrame;

    // Scene
    std::unique_ptr<Scene> scene;
    std::unique_ptr<AssetManager> assetManager;

    // Helper
    void renderFrame();
};

#endif // APPLICATION_H
