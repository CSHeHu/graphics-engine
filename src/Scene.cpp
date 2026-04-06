#include "Scene.h"

#include <glad/glad.h>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <utility>

#include "AssetManager.h"
#include "Camera.h"
#include "Config.h"
#include "Object.h"
#include "SceneDefinition.h"
#include "SceneDefinitions.h"
#include "Shader.h"
#include "TextManager.h"

Scene::Scene(AssetManager &assetManager, SceneDefinition definitionValue, TextManager *textManager)
    : assets(assetManager), textRenderer(textManager), definition(std::move(definitionValue))
{
}

Scene::~Scene()
{
    runtimeMaterials.clear();
    runtimeObjects.clear();
    activeLightSources.clear();
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
        runtimeObject.initialPosition = objectDef.position;
        runtimeObject.initialRotationAngle = object->getRotationAngle();

        runtimeObjects[objectDef.id] = runtimeObject;
    }

    // Resolve active light providers once for lit-object uniforms.
    activeLightSources.clear();
    for (const auto &entry : runtimeObjects)
    {
        const RuntimeSceneObject &runtimeObject = entry.second;
        if (runtimeObject.role == SceneRole::LightSource)
        {
            activeLightSources.push_back(runtimeObject.object);
        }
    }

    if (activeLightSources.empty())
    {
        std::cout << "Scene definition must include an object with role: LightSource" << std::endl;
        return false;
    }

    return true;
}

void Scene::update(float sceneElapsedTime)
{
    // Apply per-object behavior declared by scene definition.
    for (auto &entry : runtimeObjects)
    {
        RuntimeSceneObject &runtimeObject = entry.second;
        std::shared_ptr<Object> object = runtimeObject.object;

        if (runtimeObject.behavior == BehaviorType::Oscillate)
        {
            const float delta = std::sin(sceneElapsedTime * runtimeObject.behaviorSpeed) * runtimeObject.behaviorAmplitude;
            object->setPosition(runtimeObject.initialPosition + runtimeObject.behaviorAxis * delta);
        }
        else if (runtimeObject.behavior == BehaviorType::Spin)
        {
            object->setRotation(runtimeObject.initialRotationAngle + runtimeObject.behaviorSpeed * sceneElapsedTime,
                                runtimeObject.behaviorAxis);
        }
        else if (runtimeObject.behavior == BehaviorType::Fly)
        {
            const glm::vec3 direction = glm::normalize(runtimeObject.behaviorAxis);
            const glm::vec3 displacement = direction * (runtimeObject.behaviorSpeed * sceneElapsedTime);
            object->setPosition(runtimeObject.initialPosition + displacement);
        }
    }
}

void Scene::render(const Camera &camera, const glm::mat4 &projection, const glm::mat4 &view, float fps, float sceneElapsedTime, const UIOverlayConfig &overlayConfig, bool infoOverlayEnabled, float currentTimeSeconds)
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
        material->shader->setFloat("uTime", sceneElapsedTime);

        if (material->renderMode == RenderMode::Lit)
        {
            const int lightCount = std::min(static_cast<int>(activeLightSources.size()), MAX_LIGHT_SOURCES);

            material->shader->setVec3("objectColor", material->objectColor);
            material->shader->setVec3("viewPos", camera.Position);
            material->shader->setInt("lightCount", lightCount);

            // Backward-compatible single-light uniforms for legacy shaders.
            material->shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
            material->shader->setVec3("lightPos", activeLightSources[0]->getPosition());

            for (int i = 0; i < lightCount; ++i)
            {
                const std::string posUniform = "lightPos[" + std::to_string(i) + "]";
                const std::string colorUniform = "lightColor[" + std::to_string(i) + "]";
                material->shader->setVec3(posUniform, activeLightSources[static_cast<std::size_t>(i)]->getPosition());
                material->shader->setVec3(colorUniform, 1.0f, 1.0f, 1.0f);
            }
        }

        glBindVertexArray(object->getVAO());
        glm::mat4 model = object->getModelMatrix();
        material->shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(runtimeObject.vertexCount));
    }

    // Render scene text overlays
    if (textRenderer)
    {
        renderTextOverlay(overlayConfig, infoOverlayEnabled, fps, sceneElapsedTime, currentTimeSeconds);
    }
}

void Scene::renderTextOverlay(const UIOverlayConfig &overlayConfig, bool infoOverlayEnabled, float fps, float sceneElapsedTime, float currentTimeSeconds)
{
    for (const TextDefinition &text : definition.texts)
    {
        textRenderer->renderText(text.text, text.x, text.y, text.scale, text.color);
    }

    // Render info overlay with FPS if enabled
    if (infoOverlayEnabled && overlayConfig.enabled)
    {
        glm::vec3 overlayColor(1.0f, 1.0f, 1.0f);
        float yOffset = overlayConfig.y;

        // Dynamically render stats based on config
        for (const std::string &stat : overlayConfig.stats)
        {
            std::string statLine;

            if (stat == "fps")
            {
                statLine = "FPS: " + std::to_string(static_cast<int>(fps));
            }
            else if (stat == "sceneName")
            {
                statLine = "Scene: " + definition.name;
            }
            else if (stat == "time")
            {
                int totalMinutes = static_cast<int>(currentTimeSeconds) / 60;
                int totalSeconds = static_cast<int>(currentTimeSeconds) % 60;
                char totalTimeBuffer[32];
                snprintf(totalTimeBuffer, sizeof(totalTimeBuffer), "Total: %d:%02d", totalMinutes, totalSeconds);

                int sceneMinutes = static_cast<int>(sceneElapsedTime) / 60;
                int sceneSeconds = static_cast<int>(sceneElapsedTime) % 60;
                char sceneTimeBuffer[32];
                snprintf(sceneTimeBuffer, sizeof(sceneTimeBuffer), "Scene: %d:%02d", sceneMinutes, sceneSeconds);

                statLine = std::string(totalTimeBuffer) + " | " + std::string(sceneTimeBuffer);
            }
            else if (stat == "objects")
            {
                statLine = "Objects:";
                for (const SceneObjectDefinition &object : definition.objects)
                {
                    statLine += " " + object.id;
                }
            }
            else if (stat == "shaders")
            {
                statLine = "Shaders:";
                for (const MaterialDefinition &objectDef : definition.materials)
                {
                    statLine += " " + objectDef.id;
                }
            }

            if (!statLine.empty())
            {
                textRenderer->renderText(statLine, overlayConfig.x, yOffset, overlayConfig.scale, overlayColor);
                yOffset -= overlayConfig.lineSpacing;
            }
        }
    }
}