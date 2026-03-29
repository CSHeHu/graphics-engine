
#include "InputManager.h"

float InputManager::lastX = 1920 / 2.0f;
float InputManager::lastY = 1080 / 2.0f;
bool InputManager::firstMouse = true;
bool InputManager::cameraControlEnabled = true;
Camera *InputManager::camera = nullptr;

void InputManager::processInput(GLFWwindow *window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (!InputManager::cameraControlEnabled || InputManager::camera == nullptr)
        return;

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
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

void InputManager::scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (!cameraControlEnabled || camera == nullptr)
    {
        return;
    }

    InputManager::camera->ProcessMouseScroll(static_cast<float>(yoffset));
}