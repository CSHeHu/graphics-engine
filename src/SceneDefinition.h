#ifndef SCENEDEFINITION_H
#define SCENEDEFINITION_H

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
    float fpsX;
    float fpsY;
    float sceneNameX;
    float sceneNameY;
    float scale;
};

struct SceneDefinition
{
    std::string name;
    CameraRouteDefinition camera;
    std::vector<MaterialDefinition> materials;
    std::vector<SceneObjectDefinition> objects;
    std::vector<TextDefinition> texts;
};

#endif // SCENEDEFINITION_H
