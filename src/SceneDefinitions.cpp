#include "SceneDefinitions.h"

SceneDefinition createBasicSceneDefinition()
{
    SceneDefinition definition;
    definition.name = "basic";

    definition.objects = {
        {
            "lightCube",
            "cube.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(1.0f, 1.0f, -5.0f),
            {"assets/shaders/lightSource.vs", "assets/shaders/lightSource.fs", ""},
            "lightSource",
            glm::vec3(1.0f, 1.0f, 1.0f),
        },
        {
            "lightTargetCube",
            "cube.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(3.0f, 2.0f, -7.0f),
            {"assets/shaders/lightTarget.vs", "assets/shaders/lightTarget.fs", ""},
            "lit",
            glm::vec3(1.0f, 0.5f, 0.31f),
        },
        {
            "ground",
            "ground.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(0.0f, 0.0f, 0.0f),
            {"assets/shaders/lightTarget.vs", "assets/shaders/lightTarget.fs", ""},
            "lit",
            glm::vec3(0.2f, 0.7f, 0.2f),
        },
    };

    return definition;
}

SceneDefinition createAlternateSceneDefinition()
{
    SceneDefinition definition;
    definition.name = "alternate";

    definition.objects = {
        {
            "lightCube",
            "cube.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(-2.0f, 1.5f, -4.0f),
            {"assets/shaders/lightSource.vs", "assets/shaders/lightSource.fs", ""},
            "lightSource",
            glm::vec3(1.0f, 1.0f, 1.0f),
        },
        {
            "lightTargetCube",
            "cube.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(0.0f, 3.0f, -9.0f),
            {"assets/shaders/lightTarget.vs", "assets/shaders/lightTarget.fs", ""},
            "lit",
            glm::vec3(0.2f, 0.6f, 1.0f),
        },
        {
            "ground",
            "ground.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(0.0f, -0.5f, 0.0f),
            {"assets/shaders/lightTarget.vs", "assets/shaders/lightTarget.fs", ""},
            "lit",
            glm::vec3(0.5f, 0.45f, 0.25f),
        },
    };

    return definition;
}
