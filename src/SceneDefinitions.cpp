#include "SceneDefinitions.h"

namespace
{
    const std::vector<SceneRegistryEntry> kSceneRegistry = {
        {SceneId::Basic, createBasicSceneDefinition},
        {SceneId::Alternate, createAlternateSceneDefinition},
    };

    const std::vector<SceneCycleEntry> kDefaultSceneCycle = {
        {SceneId::Basic, 5.0f},
        {SceneId::Alternate, 3.0f},
        {SceneId::Alternate, 7.0f},
        {SceneId::Basic, 4.0f},
    };
} // namespace

SceneDefinition createBasicSceneDefinition()
{
    SceneDefinition definition;
    definition.name = "basic";

    definition.materials = {
        {"lightSourceMaterial", ShaderProgram::LightSource, RenderMode::LightSource, glm::vec3(1.0f, 1.0f, 1.0f)},
        {"litOrangeMaterial", ShaderProgram::LightTarget, RenderMode::Lit, glm::vec3(1.0f, 0.5f, 0.31f)},
        {"litGroundMaterial", ShaderProgram::LightTarget, RenderMode::Lit, glm::vec3(0.2f, 0.7f, 0.2f)},
    };

    definition.objects = {
        {
            "lightCube",
            SceneRole::LightSource,
            "cube.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(1.0f, 1.0f, -5.0f),
            "lightSourceMaterial",
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
            "litOrangeMaterial",
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
            "litGroundMaterial",
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

    definition.materials = {
        {"lightSourceMaterial", ShaderProgram::LightSource, RenderMode::LightSource, glm::vec3(1.0f, 1.0f, 1.0f)},
        {"litBlueMaterial", ShaderProgram::LightTarget, RenderMode::Lit, glm::vec3(0.2f, 0.6f, 1.0f)},
        {"litGroundMaterial", ShaderProgram::LightTarget, RenderMode::Lit, glm::vec3(0.5f, 0.45f, 0.25f)},
    };

    definition.objects = {
        {
            "lightCube",
            SceneRole::LightSource,
            "cube.obj",
            Object::VertexLayout::PositionNormal,
            glm::vec3(-2.0f, 1.5f, -4.0f),
            "lightSourceMaterial",
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
            "litBlueMaterial",
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
            "litGroundMaterial",
            BehaviorType::None,
            0.0f,
            glm::vec3(0.0f, 1.0f, 0.0f),
            0.0f,
        },
    };

    return definition;
}

const std::vector<SceneRegistryEntry> &getSceneRegistry()
{
    return kSceneRegistry;
}

const std::vector<SceneCycleEntry> &getDefaultSceneCycle()
{
    return kDefaultSceneCycle;
}

bool tryCreateSceneDefinition(SceneId id, SceneDefinition &outDefinition)
{
    for (const SceneRegistryEntry &entry : kSceneRegistry)
    {
        if (entry.id == id)
        {
            outDefinition = entry.factory();
            return true;
        }
    }

    return false;
}
