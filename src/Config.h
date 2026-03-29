#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// Window and Display Configuration
// ============================================================================
constexpr unsigned int SCREEN_WIDTH = 1920;
constexpr unsigned int SCREEN_HEIGHT = 1080;
constexpr const char* WINDOW_TITLE = "LearnOpenGL";

// ============================================================================
// OpenGL Configuration
// ============================================================================
constexpr int GL_CONTEXT_VERSION_MAJOR = 3;
constexpr int GL_CONTEXT_VERSION_MINOR = 3;
// GLFW_OPENGL_CORE_PROFILE is set in Application.cpp to use the GLFW constant

// ============================================================================
// Camera Configuration
// ============================================================================
constexpr float CAMERA_DEFAULT_YAW = -90.0f;
constexpr float CAMERA_DEFAULT_PITCH = 0.0f;
constexpr float CAMERA_DEFAULT_SPEED = 2.5f;
constexpr float CAMERA_DEFAULT_SENSITIVITY = 0.1f;
constexpr float CAMERA_DEFAULT_ZOOM = 45.0f;
constexpr float CAMERA_DEFAULT_X = 0.0f;
constexpr float CAMERA_DEFAULT_Y = 5.0f;
constexpr float CAMERA_DEFAULT_Z = 3.0f;

// ============================================================================
// Asset Paths
// ============================================================================
constexpr const char* ASSETS_BASE_PATH = "assets";
constexpr const char* SCENES_PATH = "assets/scenes";
constexpr const char* MESHES_PATH = "assets/meshes";
constexpr const char* SHADERS_PATH = "assets/shaders";
constexpr const char* SCENE_CONFIG_FILE = "assets/scenes/scene_config.json";

// ============================================================================
// Input Configuration
// ============================================================================
// Camera mode toggle key is GLFW_KEY_C (defined as 67 in GLFW)
// This is handled in InputManager.cpp with glfwGetKey(window, GLFW_KEY_C)

// ============================================================================
// Rendering Configuration
// ============================================================================
constexpr std::size_t POSITION_NORMAL_STRIDE = 6;  // Position (3) + Normal (3)
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 100.0f;

// ============================================================================
// Physics / Behavior Configuration
// ============================================================================
// These can be extended as needed for animation speeds, etc.

#endif // CONFIG_H
