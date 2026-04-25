
#include "InputManager.h"

#include <glad/glad.h>

#include "SceneDefinitions.h"

InputManager* InputManager::callbackTarget = nullptr;

InputManager::InputManager()
    : lastX(0.0f), lastY(0.0f), firstMouse(true), cameraControlEnabled(true),
      cameraModeToggleLatch(false), infoOverlayToggleLatch(false),
      pauseToggleLatch(false), stepForwardLatch(false), stepBackwardLatch(false),
      pendingActions{false, false, false, false, false}, camera(nullptr)
{
}

InputManager::~InputManager()
{
    if (callbackTarget == this)
    {
        callbackTarget = nullptr;
    }
}

void InputManager::registerAsCallbackTarget()
{
    callbackTarget = this;
}

void InputManager::processInput(GLFWwindow* window, float deltaTime)
{
    const InputConfig& input = SceneDefinitions::getRuntimeConfig().input;

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

    if (!cameraControlEnabled || camera == nullptr)
        return;

    // Camera movement
    if (glfwGetKey(window, input.keyMoveForward) == GLFW_PRESS)
        camera->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, input.keyMoveBackward) == GLFW_PRESS)
        camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, input.keyMoveLeft) == GLFW_PRESS)
        camera->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, input.keyMoveRight) == GLFW_PRESS)
        camera->ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, input.keyMoveUp) == GLFW_PRESS)
        camera->ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, input.keyMoveDown) == GLFW_PRESS)
        camera->ProcessKeyboard(DOWN, deltaTime);
}

void InputManager::framebufferSizeCallback(GLFWwindow* window, int width,
                                           int height)
{
    (void)window;
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void InputManager::mouseCallback(GLFWwindow* window, double xposIn,
                                 double yposIn)
{
    (void)window;
    if (callbackTarget == nullptr)
    {
        return;
    }

    if (!callbackTarget->cameraControlEnabled || callbackTarget->camera == nullptr)
    {
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (callbackTarget->firstMouse)
    {
        callbackTarget->lastX = xpos;
        callbackTarget->lastY = ypos;
        callbackTarget->firstMouse = false;
    }

    float xoffset = xpos - callbackTarget->lastX;
    float yoffset = callbackTarget->lastY - ypos; // reversed since y-coordinates go from bottom to top

    callbackTarget->lastX = xpos;
    callbackTarget->lastY = ypos;

    callbackTarget->camera->ProcessMouseMovement(xoffset, yoffset);
}

void InputManager::setCamera(Camera* cameraPtr)
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

void InputManager::scrollCallback(GLFWwindow* window, double xoffset,
                                  double yoffset)
{
    (void)window;
    (void)xoffset;
    if (callbackTarget == nullptr)
    {
        return;
    }

    if (!callbackTarget->cameraControlEnabled || callbackTarget->camera == nullptr)
    {
        return;
    }

    callbackTarget->camera->ProcessMouseScroll(static_cast<float>(yoffset));
}