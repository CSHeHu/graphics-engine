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
    runtimeObjects.clear();
    activeLightSource.reset();
}

bool Scene::init()
{
    runtimeObjects.clear();

    for (const SceneObjectDefinition &objectDef : definition.objects)
    {
        const std::vector<float> &vertices = assets.getMeshVertices(objectDef.meshName);
        const std::size_t vertexCount = assets.getMeshVertexCount(objectDef.meshName);

        std::shared_ptr<Shader> shader = assets.getShader(
            objectDef.shader.vertex,
            objectDef.shader.fragment,
            objectDef.shader.geometry);

        std::shared_ptr<Object> object = std::make_shared<Object>(
            shader,
            vertices,
            objectDef.position,
            objectDef.layout);

        RuntimeSceneObject runtimeObject;
        runtimeObject.object = object;
        runtimeObject.vertexCount = vertexCount;
        runtimeObject.role = objectDef.role;
        runtimeObject.renderMode = objectDef.renderMode;
        runtimeObject.objectColor = objectDef.objectColor;
        runtimeObject.behavior = objectDef.behavior;
        runtimeObject.behaviorSpeed = objectDef.behaviorSpeed;
        runtimeObject.behaviorAxis = objectDef.behaviorAxis;
        runtimeObject.behaviorAmplitude = objectDef.behaviorAmplitude;

        runtimeObjects[objectDef.id] = runtimeObject;
    }

    activeLightSource.reset();
    for (const auto &entry : runtimeObjects)
    {
        const RuntimeSceneObject &runtimeObject = entry.second;
        if (runtimeObject.role == "LightSource")
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

    for (auto &entry : runtimeObjects)
    {
        RuntimeSceneObject &runtimeObject = entry.second;
        std::shared_ptr<Object> object = runtimeObject.object;

        if (runtimeObject.behavior == "oscillate")
        {
            const float delta = std::sin(elapsedTime * runtimeObject.behaviorSpeed) * runtimeObject.behaviorAmplitude;
            object->setPosition(object->getPosition() + delta);
        }
        else if (runtimeObject.behavior == "spin")
        {
            object->rotate(runtimeObject.behaviorSpeed * deltaTime, runtimeObject.behaviorAxis);
        }
    }
}

void Scene::render(const Camera &camera, const glm::mat4 &projection, const glm::mat4 &view)
{
    for (const auto &entry : runtimeObjects)
    {
        const RuntimeSceneObject &runtimeObject = entry.second;
        std::shared_ptr<Object> object = runtimeObject.object;

        object->shader->use();
        object->shader->setMat4("projection", projection);
        object->shader->setMat4("view", view);

        if (runtimeObject.renderMode == "lit")
        {
            object->shader->setVec3("objectColor", runtimeObject.objectColor);
            object->shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
            object->shader->setVec3("viewPos", camera.Position);
            object->shader->setVec3("lightPos", activeLightSource->getPosition());
        }

        glBindVertexArray(object->getVAO());
        glm::mat4 model = object->getModelMatrix();
        object->shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(runtimeObject.vertexCount));
    }
}
