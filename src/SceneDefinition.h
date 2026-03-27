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

enum class ShaderProgram
{
    LightSource,
    LightTarget,
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
};

struct SceneObjectDefinition
{
    std::string id;
    SceneRole role;
    std::string meshName;
    Object::VertexLayout layout;
    glm::vec3 position;

    ShaderProgram shaderProgram;

    RenderMode renderMode;
    glm::vec3 objectColor;

    BehaviorType behavior;
    float behaviorSpeed;
    glm::vec3 behaviorAxis;
    float behaviorAmplitude;
};

struct SceneDefinition
{
    std::string name;
    std::vector<SceneObjectDefinition> objects;
};

#endif // SCENEDEFINITION_H
