#ifndef SCENEDEFINITION_H
#define SCENEDEFINITION_H

#include <string>
#include <vector>
#include <glm.hpp>

#include "Object.h"

struct ShaderPaths
{
    std::string vertex;
    std::string fragment;
    std::string geometry;
};

struct SceneObjectDefinition
{
    std::string id;
    std::string meshName;
    Object::VertexLayout layout;
    glm::vec3 position;

    ShaderPaths shader;

    // Render mode values currently used by Scene: "lightSource", "lit"
    std::string renderMode;
    glm::vec3 objectColor;
};

struct SceneDefinition
{
    std::string name;
    std::vector<SceneObjectDefinition> objects;
};

#endif // SCENEDEFINITION_H
