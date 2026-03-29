#include "Scene.h"

#include <glad/glad.h>
#include <cmath>
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <utility>

#include "AssetManager.h"
#include "Camera.h"
#include "Object.h"
#include "SceneDefinition.h"
#include "Shader.h"

Scene::Scene(AssetManager &assetManager, SceneDefinition definitionValue)
    : assets(assetManager), definition(std::move(definitionValue)), elapsedTime(0.0f)
{
}

Scene::~Scene()
{
    runtimeMaterials.clear();
    runtimeObjects.clear();
    activeLightSource.reset();
}

bool Scene::init()
{
    runtimeMaterials.clear();
    runtimeObjects.clear();

    for (const MaterialDefinition &materialDef : definition.materials)
    {
        std::shared_ptr<RuntimeMaterial> material = std::make_shared<RuntimeMaterial>();
        material->shader = assets.getShader(
            materialDef.vertexShaderPath,
            materialDef.fragmentShaderPath,
            materialDef.geometryShaderPath);
        material->renderMode = materialDef.renderMode;
        material->objectColor = materialDef.objectColor;

        runtimeMaterials[materialDef.id] = material;
    }

    // Build runtime objects from scene definition data.
    for (const SceneObjectDefinition &objectDef : definition.objects)
    {
        const auto materialIt = runtimeMaterials.find(objectDef.materialId);
        if (materialIt == runtimeMaterials.end())
        {
            std::cout << "Scene object references unknown material id for object: " << objectDef.id << std::endl;
            return false;
        }

        const std::vector<float> &vertices = assets.getMeshVertices(objectDef.meshName);
        const std::size_t vertexCount = assets.getMeshVertexCount(objectDef.meshName);
        std::shared_ptr<RuntimeMaterial> material = materialIt->second;

        std::shared_ptr<Object> object = std::make_shared<Object>(
            material->shader,
            vertices,
            objectDef.position,
            objectDef.scale,
            objectDef.layout);

        // Apply initial rotations (pitch, yaw, roll)
        if (objectDef.rotation.y != 0.0f)
        {
            object->setRotation(objectDef.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        if (objectDef.rotation.x != 0.0f)
        {
            object->rotate(objectDef.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        }
        if (objectDef.rotation.z != 0.0f)
        {
            object->rotate(objectDef.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        }

        RuntimeSceneObject runtimeObject;
        runtimeObject.object = object;
        runtimeObject.material = material;
        runtimeObject.vertexCount = vertexCount;
        runtimeObject.role = objectDef.role;
        runtimeObject.behavior = objectDef.behavior;
        runtimeObject.behaviorSpeed = objectDef.behaviorSpeed;
        runtimeObject.behaviorAxis = objectDef.behaviorAxis;
        runtimeObject.behaviorAmplitude = objectDef.behaviorAmplitude;

        runtimeObjects[objectDef.id] = runtimeObject;
    }

    // Resolve the active light provider once for lit-object uniforms.
    activeLightSource.reset();
    for (const auto &entry : runtimeObjects)
    {
        const RuntimeSceneObject &runtimeObject = entry.second;
        if (runtimeObject.role == SceneRole::LightSource)
        {
            activeLightSource = runtimeObject.object;
            break;
        }
    }

    if (!activeLightSource)
    {
        std::cout << "Scene definition must include an object with role: LightSource" << std::endl;
        return false;
    }

    return true;
}

void Scene::update(float deltaTime)
{
    elapsedTime += deltaTime;

    // Apply per-object behavior declared by scene definition.
    for (auto &entry : runtimeObjects)
    {
        RuntimeSceneObject &runtimeObject = entry.second;
        std::shared_ptr<Object> object = runtimeObject.object;

        if (runtimeObject.behavior == BehaviorType::Oscillate)
        {
            const float delta = std::sin(elapsedTime * runtimeObject.behaviorSpeed) * runtimeObject.behaviorAmplitude;
            object->setPosition(object->getPosition() + delta);
        }
        else if (runtimeObject.behavior == BehaviorType::Spin)
        {
            object->rotate(runtimeObject.behaviorSpeed * deltaTime, runtimeObject.behaviorAxis);
        }
        else if (runtimeObject.behavior == BehaviorType::Fly)
        {
            // Move along the direction specified by behaviorAxis at speed behaviorSpeed
            const glm::vec3 direction = glm::normalize(runtimeObject.behaviorAxis);
            const glm::vec3 displacement = direction * (runtimeObject.behaviorSpeed * deltaTime);
            object->setPosition(object->getPosition() + displacement);
        }
    }
}

void Scene::render(const Camera &camera, const glm::mat4 &projection, const glm::mat4 &view)
{
    // Render all objects with shared camera matrices and mode-specific uniforms.
    for (const auto &entry : runtimeObjects)
    {
        const RuntimeSceneObject &runtimeObject = entry.second;
        std::shared_ptr<Object> object = runtimeObject.object;

        std::shared_ptr<RuntimeMaterial> material = runtimeObject.material;
        material->shader->use();
        material->shader->setMat4("projection", projection);
        material->shader->setMat4("view", view);

        if (material->renderMode == RenderMode::Lit)
        {
            material->shader->setVec3("objectColor", material->objectColor);
            material->shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
            material->shader->setVec3("viewPos", camera.Position);
            material->shader->setVec3("lightPos", activeLightSource->getPosition());
        }

        glBindVertexArray(object->getVAO());
        glm::mat4 model = object->getModelMatrix();
        material->shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(runtimeObject.vertexCount));
    }
}
