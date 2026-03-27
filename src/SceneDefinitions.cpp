#include "SceneDefinitions.h"

SceneDefinition createBasicSceneDefinition()
{
    SceneDefinition definition;
    definition.name = "basic";

    definition.objects = {
        {
            "lightCube",
            SceneRole::LightSource,
            "cube.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(1.0f, 1.0f, -5.0f),
            ShaderProgram::LightSource,
            RenderMode::LightSource,
            glm::vec3(1.0f, 1.0f, 1.0f),
            BehaviorType::Oscillate,
            1.0f,
            glm::vec3(1.0f, 1.0f, 1.0f),
            0.01f,
        },
        {
            "lightTargetCube",
            SceneRole::LightTarget,
            "cube.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(3.0f, 2.0f, -7.0f),
            ShaderProgram::LightTarget,
            RenderMode::Lit,
            glm::vec3(1.0f, 0.5f, 0.31f),
            BehaviorType::Spin,
            glm::radians(20.0f),
            glm::vec3(1.0f, 0.0f, 1.0f),
            0.0f,
        },
        {
            "ground",
            SceneRole::Ground,
            "ground.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(0.0f, 0.0f, 0.0f),
            ShaderProgram::LightTarget,
            RenderMode::Lit,
            glm::vec3(0.2f, 0.7f, 0.2f),
            BehaviorType::None,
            0.0f,
            glm::vec3(0.0f, 1.0f, 0.0f),
            0.0f,
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
            SceneRole::LightSource,
            "cube.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(-2.0f, 1.5f, -4.0f),
            ShaderProgram::LightSource,
            RenderMode::LightSource,
            glm::vec3(1.0f, 1.0f, 1.0f),
            BehaviorType::Oscillate,
            1.0f,
            glm::vec3(1.0f, 1.0f, 1.0f),
            0.01f,
        },
        {
            "lightTargetCube",
            SceneRole::LightTarget,
            "cube.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(0.0f, 3.0f, -9.0f),
            ShaderProgram::LightTarget,
            RenderMode::Lit,
            glm::vec3(0.2f, 0.6f, 1.0f),
            BehaviorType::Spin,
            glm::radians(20.0f),
            glm::vec3(1.0f, 0.0f, 1.0f),
            0.0f,
        },
        {
            "ground",
            SceneRole::Ground,
            "ground.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(0.0f, -0.5f, 0.0f),
            ShaderProgram::LightTarget,
            RenderMode::Lit,
            glm::vec3(0.5f, 0.45f, 0.25f),
            BehaviorType::None,
            0.0f,
            glm::vec3(0.0f, 1.0f, 0.0f),
            0.0f,
        },
    };

    return definition;
}
