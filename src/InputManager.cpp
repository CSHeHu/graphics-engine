
#include "InputManager.h"

#include "Config.h"

float InputManager::lastX = static_cast<float>(SCREEN_WIDTH) / 2.0f;
float InputManager::lastY = static_cast<float>(SCREEN_HEIGHT) / 2.0f;
bool InputManager::firstMouse = true;
bool InputManager::cameraControlEnabled = true;
bool InputManager::cameraModeToggleLatch = false;
bool InputManager::infoOverlayToggleLatch = false;
bool InputManager::cameraModeToggleRequested = false;
bool InputManager::infoOverlayToggleRequested = false;
Camera *InputManager::camera = nullptr;

void InputManager::processInput(GLFWwindow *window, float deltaTime)
{
    if (glfwGetKey(window, KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const bool cameraTogglePressed = glfwGetKey(window, KEY_TOGGLE_CAMERA_MODE) == GLFW_PRESS;
    if (cameraTogglePressed && !cameraModeToggleLatch)
    {
        cameraModeToggleRequested = true;
    }
    cameraModeToggleLatch = cameraTogglePressed;

    const bool infoOverlayTogglePressed = glfwGetKey(window, KEY_TOGGLE_INFO_OVERLAY) == GLFW_PRESS;
    if (infoOverlayTogglePressed && !infoOverlayToggleLatch)
    {
        infoOverlayToggleRequested = true;
    }
    infoOverlayToggleLatch = infoOverlayTogglePressed;

    if (!InputManager::cameraControlEnabled || InputManager::camera == nullptr)
        return;

    // Camera movement
    if (glfwGetKey(window, KEY_MOVE_FORWARD) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, KEY_MOVE_BACKWARD) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, KEY_MOVE_LEFT) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, KEY_MOVE_RIGHT) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, KEY_MOVE_UP) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, KEY_MOVE_DOWN) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(DOWN, deltaTime);
}

void InputManager::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void InputManager::mouseCallback(GLFWwindow *window, double xposIn, double yposIn)
{
    if (!cameraControlEnabled || camera == nullptr)
    {
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera->ProcessMouseMovement(xoffset, yoffset);
}

void InputManager::setCamera(Camera *cameraPtr)
{
    camera = cameraPtr;
}

void InputManager::setCameraControlEnabled(bool enabled)
{
    cameraControlEnabled = enabled;
    firstMouse = true;
}

bool InputManager::consumeCameraModeToggleRequest()
{
    const bool wasRequested = cameraModeToggleRequested;
    cameraModeToggleRequested = false;
    return wasRequested;
}

bool InputManager::consumeInfoOverlayToggleRequest()
{
    const bool wasRequested = infoOverlayToggleRequested;
    infoOverlayToggleRequested = false;
    return wasRequested;
}

void InputManager::scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (!cameraControlEnabled || camera == nullptr)
    {
        return;
    }

    InputManager::camera->ProcessMouseScroll(static_cast<float>(yoffset));
}