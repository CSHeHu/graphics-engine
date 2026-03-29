#define GLFW_INCLUDE_NONE
#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <GLFW/glfw3.h>
#include <functional>

#include "Camera.h"

class InputManager
{
public:
	using CameraModeToggleCallback = std::function<void()>;

	static void setCamera(Camera *cameraPtr);
	static void setCameraControlEnabled(bool enabled);
	static void setCameraModeToggleCallback(CameraModeToggleCallback callback);
	static void processInput(GLFWwindow *window, float deltaTime);

	static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
	static void mouseCallback(GLFWwindow *window, double xposIn, double yposIn);
	static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

private:
	static float lastX;
	static float lastY;
	static bool firstMouse;
	static bool cameraControlEnabled;
	static bool cameraModeToggleLatch;
	static Camera *camera;
	static CameraModeToggleCallback cameraModeToggleCallback;
};

#endif
