#ifndef SCENEDEFINITION_H
#define SCENEDEFINITION_H

#include <string>
#include <vector>
#include <glm.hpp>

#include "Config.h"
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
};

struct ShadowConfig
{
    bool enabled = false;
    int mapSize = SHADOW_MAP_SIZE_DEFAULT;
    float orthoSize = SHADOW_ORTHO_SIZE_DEFAULT;
    float fovDegrees = SHADOW_FOV_DEGREES_DEFAULT;
    float nearPlane = SHADOW_NEAR_PLANE_DEFAULT;
    float farPlane = SHADOW_FAR_PLANE_DEFAULT;
    float biasMin = SHADOW_BIAS_MIN_DEFAULT;
    float biasSlope = SHADOW_BIAS_SLOPE_DEFAULT;
};

struct SceneDefinition
{
    std::string name;
    CameraRouteDefinition camera;
    ShadowConfig shadows;
    /** Material definitions referenced by scene objects. */
    std::vector<MaterialDefinition> materials;
    /** Scene object definitions instantiated for runtime rendering. */
    std::vector<SceneObjectDefinition> objects;
    /** Optional static text overlays rendered with scene. */
    std::vector<TextDefinition> texts;
};

#endif // SCENEDEFINITION_H
