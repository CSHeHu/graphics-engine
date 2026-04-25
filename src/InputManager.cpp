
#include "InputManager.h"

#include "SceneDefinitions.h"

float InputManager::lastX = 0.0f;
float InputManager::lastY = 0.0f;
bool InputManager::firstMouse = true;
bool InputManager::cameraControlEnabled = true;
bool InputManager::cameraModeToggleLatch = false;
bool InputManager::infoOverlayToggleLatch = false;
bool InputManager::pauseToggleLatch = false;
bool InputManager::stepForwardLatch = false;
bool InputManager::stepBackwardLatch = false;
InputActions InputManager::pendingActions = {false, false, false, false,
                                             false};
Camera *InputManager::camera = nullptr;

void InputManager::processInput(GLFWwindow *window, float deltaTime)
{
    const InputConfig &input = SceneDefinitions::getRuntimeConfig().input;

    if (glfwGetKey(window, input.keyEscape) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const bool cameraTogglePressed = glfwGetKey(window, input.keyToggleCameraMode) == GLFW_PRESS;
    if (cameraTogglePressed && !cameraModeToggleLatch)
    {
        pendingActions.toggleCameraMode = true;
    }
    cameraModeToggleLatch = cameraTogglePressed;

    const bool infoOverlayTogglePressed = glfwGetKey(window, input.keyToggleInfoOverlay) == GLFW_PRESS;
    if (infoOverlayTogglePressed && !infoOverlayToggleLatch)
    {
        pendingActions.toggleInfoOverlay = true;
    }
    infoOverlayToggleLatch = infoOverlayTogglePressed;

    const bool pauseTogglePressed = glfwGetKey(window, input.keyTogglePause) == GLFW_PRESS;
    if (pauseTogglePressed && !pauseToggleLatch)
    {
        pendingActions.togglePause = true;
    }
    pauseToggleLatch = pauseTogglePressed;

    const bool stepForwardPressed = glfwGetKey(window, input.keyStepTimeForward) == GLFW_PRESS;
    if (stepForwardPressed && !stepForwardLatch)
    {
        pendingActions.stepTimeForward = true;
    }
    stepForwardLatch = stepForwardPressed;

    const bool stepBackwardPressed = glfwGetKey(window, input.keyStepTimeBackward) == GLFW_PRESS;
    if (stepBackwardPressed && !stepBackwardLatch)
    {
        pendingActions.stepTimeBackward = true;
    }
    stepBackwardLatch = stepBackwardPressed;

    if (!InputManager::cameraControlEnabled || InputManager::camera == nullptr)
        return;

    // Camera movement
    if (glfwGetKey(window, input.keyMoveForward) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, input.keyMoveBackward) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, input.keyMoveLeft) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, input.keyMoveRight) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, input.keyMoveUp) == GLFW_PRESS)
        InputManager::camera->ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, input.keyMoveDown) == GLFW_PRESS)
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

InputActions InputManager::consumeActions()
{
    const InputActions actions = pendingActions;
    pendingActions = {false, false, false, false, false};
    return actions;
}

void InputManager::scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (!cameraControlEnabled || camera == nullptr)
    {
        return;
    }

    InputManager::camera->ProcessMouseScroll(static_cast<float>(yoffset));
}