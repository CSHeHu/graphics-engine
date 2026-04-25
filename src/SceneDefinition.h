#ifndef SCENEDEFINITION_H
#define SCENEDEFINITION_H

#include <cstddef>
#include <string>
#include <vector>
#include <glm.hpp>

#include "Object.h"

enum class SceneRole
{
    None,
    LightSource,
    LightTarget,
    Ground,
};

enum class RenderMode
{
    LightSource,
    Lit,
};

enum class BehaviorType
{
    None,
    Oscillate,
    Spin,
    Fly,
};

enum class CameraMode
{
    Manual,
    Scripted,
};

enum class WindowMode
{
    Windowed,
    Fullscreen,
};

struct CameraKeyframe
{
    float timeSeconds;
    glm::vec3 position;
    glm::vec3 lookAt;
};

struct CameraRouteDefinition
{
    CameraMode mode = CameraMode::Manual;
    bool loop = true;
    /** Time-ordered camera keyframes for scripted mode. */
    std::vector<CameraKeyframe> keyframes;
};

struct MaterialDefinition
{
    std::string id;
    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    std::string geometryShaderPath;
    RenderMode renderMode;
    glm::vec3 objectColor;
};

struct SceneObjectDefinition
{
    std::string id;
    SceneRole role;
    std::string meshName;
    Object::VertexLayout layout;
    glm::vec3 position;
    glm::vec3 rotation; // Euler angles in radians (pitch, yaw, roll)
    glm::vec3 scale;
    std::string materialId;

    BehaviorType behavior;
    float behaviorSpeed;
    glm::vec3 behaviorAxis;
    float behaviorAmplitude;

    glm::vec3 lightColor;
    float lightIntensity;
};

struct TextDefinition
{
    std::string text;
    float x;
    float y;
    float scale;
    glm::vec3 color;
};

struct UIOverlayConfig
{
    bool enabled;
    std::string fontPath;
    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    float x;
    float y;
    float scale;
    float lineSpacing;
    /** Overlay stat identifiers to render in configured order. */
    std::vector<std::string> stats;
};

struct WindowConfig
{
    WindowMode mode;
    int width;
    int height;
    bool cursorCaptured;
    bool vsyncEnabled;
};

struct OpenGLConfig
{
    int contextVersionMajor;
    int contextVersionMinor;
};

struct CameraDefaultsConfig
{
    float yaw;
    float pitch;
    float speed;
    float sensitivity;
    float zoom;
    glm::vec3 position;
};

struct AssetPathsConfig
{
    std::string scenesPath;
    std::string meshesPath;
    std::string shadersPath;
};

struct InputConfig
{
    bool cameraControlsEnabled;
    int keyEscape;
    int keyMoveForward;
    int keyMoveBackward;
    int keyMoveLeft;
    int keyMoveRight;
    int keyMoveUp;
    int keyMoveDown;
    int keyToggleCameraMode;
    int keyToggleInfoOverlay;
    int keyTogglePause;
    int keyStepTimeBackward;
    int keyStepTimeForward;
};

struct RenderingConfig
{
    std::size_t positionNormalStride;
    int maxLightSources;
    float nearPlane;
    float farPlane;
    bool depthTestEnabled;
    bool blendEnabled;
};

struct RuntimeConfig
{
    std::string windowTitle;
    OpenGLConfig opengl;
    CameraDefaultsConfig camera;
    AssetPathsConfig assets;
    InputConfig input;
    RenderingConfig rendering;
};

struct SceneDefinition
{
    std::string name;
    CameraRouteDefinition camera;
    /** Material definitions referenced by scene objects. */
    std::vector<MaterialDefinition> materials;
    /** Scene object definitions instantiated for runtime rendering. */
    std::vector<SceneObjectDefinition> objects;
    /** Optional static text overlays rendered with scene. */
    std::vector<TextDefinition> texts;
};

#endif // SCENEDEFINITION_H
