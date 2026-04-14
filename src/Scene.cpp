#include "Scene.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <glm.hpp>
#include <iostream>
#include <unordered_set>
#include <utility>

#include "AssetManager.h"
#include "Camera.h"
#include "Object.h"
#include "SceneDefinition.h"
#include "SceneDefinitions.h"
#include "Shader.h"
#include "ShadowManager.h"
#include "TextManager.h"

Scene::Scene(AssetManager& assetManager, SceneDefinition definitionValue,
             TextManager* textManager)
    : assets(assetManager), textRenderer(textManager),
      definition(std::move(definitionValue)),
      shadowManager(std::make_unique<ShadowManager>())
{
}

Scene::~Scene()
{
    if (shadowManager)
    {
        shadowManager->shutdown();
    }
    runtimeMaterials.clear();
    runtimeObjects.clear();
    activeLightSources.clear();
}

bool Scene::init()
{
    runtimeMaterials.clear();
    runtimeObjects.clear();

    for (const MaterialDefinition& materialDef : definition.materials)
    {
        std::shared_ptr<RuntimeMaterial> material =
            std::make_shared<RuntimeMaterial>();
        material->shader     = assets.getShader(materialDef.vertexShaderPath,
                                                materialDef.fragmentShaderPath,
                                                materialDef.geometryShaderPath);
        material->renderMode = materialDef.renderMode;
        material->objectColor = materialDef.objectColor;

        runtimeMaterials[materialDef.id] = material;
    }

    // Build runtime objects from scene definition data.
    for (const SceneObjectDefinition& objectDef : definition.objects)
    {
        const auto materialIt = runtimeMaterials.find(objectDef.materialId);
        if (materialIt == runtimeMaterials.end())
        {
            std::cout
                << "Scene object references unknown material id for object: "
                << objectDef.id << std::endl;
            return false;
        }

        const std::vector<float>& vertices =
            assets.getMeshVertices(objectDef.meshName);
        const std::size_t vertexCount =
            assets.getMeshVertexCount(objectDef.meshName);
        std::shared_ptr<RuntimeMaterial> material = materialIt->second;

        std::shared_ptr<Object> object = std::make_shared<Object>(
            material->shader, vertices, objectDef.position, objectDef.scale,
            objectDef.layout);

        // Apply initial rotations (pitch, yaw, roll)
        if (objectDef.rotation.y != 0.0f)
        {
            object->setRotation(objectDef.rotation.y,
                                glm::vec3(0.0f, 1.0f, 0.0f));
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
        runtimeObject.object               = object;
        runtimeObject.material             = material;
        runtimeObject.vertexCount          = vertexCount;
        runtimeObject.role                 = objectDef.role;
        runtimeObject.behavior             = objectDef.behavior;
        runtimeObject.behaviorSpeed        = objectDef.behaviorSpeed;
        runtimeObject.behaviorAxis         = objectDef.behaviorAxis;
        runtimeObject.behaviorAmplitude    = objectDef.behaviorAmplitude;
        runtimeObject.initialPosition      = objectDef.position;
        runtimeObject.initialRotationAngle = object->getRotationAngle();
        runtimeObject.lightColor           = objectDef.lightColor;
        runtimeObject.lightIntensity       = objectDef.lightIntensity;

        runtimeObjects[objectDef.id] = runtimeObject;
    }

    // Resolve active light providers once for lit-object uniforms.
    activeLightSources.clear();
    for (auto& entry : runtimeObjects)
    {
        RuntimeSceneObject& runtimeObject = entry.second;
        if (runtimeObject.role == SceneRole::LightSource)
        {
            activeLightSources.push_back(&runtimeObject);
        }
    }

    if (activeLightSources.empty())
    {
        std::cout
            << "Scene definition must include an object with role: LightSource"
            << std::endl;
        return false;
    }

    if (definition.shadows.enabled &&
        !shadowManager->init(assets, definition.shadows))
    {
        std::cout << "Failed to initialize shadow resources" << std::endl;
        return false;
    }
    if (!definition.shadows.enabled)
    {
        shadowManager->shutdown();
    }

    return true;
}

void Scene::update(float sceneElapsedTime)
{
    // Apply per-object behavior declared by scene definition.
    for (auto& entry : runtimeObjects)
    {
        RuntimeSceneObject&     runtimeObject = entry.second;
        std::shared_ptr<Object> object        = runtimeObject.object;

        if (runtimeObject.behavior == BehaviorType::Oscillate)
        {
            const float delta =
                std::sin(sceneElapsedTime * runtimeObject.behaviorSpeed) *
                runtimeObject.behaviorAmplitude;
            object->setPosition(runtimeObject.initialPosition +
                                runtimeObject.behaviorAxis * delta);
        }
        else if (runtimeObject.behavior == BehaviorType::Spin)
        {
            object->setRotation(runtimeObject.initialRotationAngle +
                                    runtimeObject.behaviorSpeed *
                                        sceneElapsedTime,
                                runtimeObject.behaviorAxis);
        }
        else if (runtimeObject.behavior == BehaviorType::Fly)
        {
            const glm::vec3 direction =
                glm::normalize(runtimeObject.behaviorAxis);
            const glm::vec3 displacement =
                direction * (runtimeObject.behaviorSpeed * sceneElapsedTime);
            object->setPosition(runtimeObject.initialPosition + displacement);
        }
    }
}

void Scene::render(const Camera& camera, const glm::mat4& projection,
                   const glm::mat4& view, float fps, float sceneElapsedTime,
                   const UIOverlayConfig& overlayConfig,
                   bool infoOverlayEnabled, float currentTimeSeconds)
{
    // Runtime rendering limits and bindings are centralized in
    // scene_config.json.
    const RuntimeConfig& runtimeConfig = SceneDefinitions::getRuntimeConfig();
    const int            maxLightSources =
        std::max(runtimeConfig.rendering.maxLightSources, 1);
    const int shadowMapTextureUnit =
        runtimeConfig.rendering.shadowMapTextureUnit;
    const unsigned int shadowUpdateIntervalFrames = std::max(
        runtimeConfig.rendering.shadowDefaults.updateIntervalFrames, 1u);

    const int lightCount =
        std::min(static_cast<int>(activeLightSources.size()), maxLightSources);
    std::vector<glm::vec3> lightPositions(
        static_cast<std::size_t>(maxLightSources), glm::vec3(0.0f));
    std::vector<glm::vec3> lightColors(
        static_cast<std::size_t>(maxLightSources), glm::vec3(0.0f));
    for (int i = 0; i < lightCount; ++i)
    {
        const RuntimeSceneObject* light =
            activeLightSources[static_cast<std::size_t>(i)];
        lightPositions[static_cast<std::size_t>(i)] =
            light->object->getPosition();
        lightColors[static_cast<std::size_t>(i)] =
            light->lightColor * light->lightIntensity;
    }

    if (definition.shadows.enabled && shadowManager && shadowManager->isReady())
    {
        const glm::vec3 primaryLightPosition = lightPositions[0];
        if (shadowManager->beginDepthPass(primaryLightPosition,
                                          shadowUpdateIntervalFrames))
        {
            for (const auto& entry : runtimeObjects)
            {
                const RuntimeSceneObject& runtimeObject = entry.second;
                if (runtimeObject.role == SceneRole::LightSource ||
                    runtimeObject.role == SceneRole::Ground)
                {
                    continue;
                }

                shadowManager->submitDepthRenderable(
                    runtimeObject.object->getModelMatrix(),
                    runtimeObject.object->getVAO(),
                    static_cast<int>(runtimeObject.vertexCount));
            }
            shadowManager->endDepthPass();
        }
    }

    if (definition.shadows.enabled && shadowManager && shadowManager->isReady())
    {
        shadowManager->bindShadowTexture(shadowMapTextureUnit);
    }

    static std::vector<std::string> lightPosUniformNames;
    static std::vector<std::string> lightColorUniformNames;
    static int                      cachedUniformLightCapacity = 0;
    if (cachedUniformLightCapacity != maxLightSources)
    {
        lightPosUniformNames.resize(static_cast<std::size_t>(maxLightSources));
        lightColorUniformNames.resize(
            static_cast<std::size_t>(maxLightSources));
        for (int i = 0; i < maxLightSources; ++i)
        {
            const std::size_t idx     = static_cast<std::size_t>(i);
            lightPosUniformNames[idx] = "lightPos[" + std::to_string(i) + "]";
            lightColorUniformNames[idx] =
                "lightColor[" + std::to_string(i) + "]";
        }
        cachedUniformLightCapacity = maxLightSources;
    }

    std::unordered_set<unsigned int> configuredLitPrograms;
    std::unordered_set<unsigned int> configuredUnlitPrograms;

    // Render all objects with shared camera matrices and mode-specific
    // uniforms.
    for (const auto& entry : runtimeObjects)
    {
        const RuntimeSceneObject& runtimeObject = entry.second;
        std::shared_ptr<Object>   object        = runtimeObject.object;

        std::shared_ptr<RuntimeMaterial> material = runtimeObject.material;
        material->shader->use();
        const unsigned int shaderProgramId = material->shader->ID;

        if (material->renderMode == RenderMode::Lit)
        {
            const bool firstUseThisFrame =
                configuredLitPrograms.insert(shaderProgramId).second;
            if (firstUseThisFrame)
            {
                material->shader->setMat4("projection", projection);
                material->shader->setMat4("view", view);
                material->shader->setFloat("uTime", sceneElapsedTime);
                material->shader->setVec3("viewPos", camera.getPosition());
                material->shader->setInt("lightCount", lightCount);
                material->shader->setInt("shadowEnabled",
                                         definition.shadows.enabled ? 1 : 0);
                material->shader->setFloat("shadowBiasMin",
                                           definition.shadows.biasMin);
                material->shader->setFloat("shadowBiasSlope",
                                           definition.shadows.biasSlope);
                material->shader->setFloat("shadowFarPlane",
                                           definition.shadows.farPlane);
                material->shader->setInt("shadowMap", shadowMapTextureUnit);

                if (lightCount > 0)
                {
                    // Backward-compatible single-light uniforms for legacy
                    // shaders.
                    material->shader->setVec3("lightColor", lightColors[0]);
                    material->shader->setVec3("lightPos", lightPositions[0]);
                }

                for (int i = 0; i < lightCount; ++i)
                {
                    const std::size_t idx = static_cast<std::size_t>(i);
                    material->shader->setVec3(lightPosUniformNames[idx],
                                              lightPositions[idx]);
                    material->shader->setVec3(lightColorUniformNames[idx],
                                              lightColors[idx]);
                }
            }

            material->shader->setVec3("objectColor", material->objectColor);
        }
        else if (material->renderMode == RenderMode::LightSource)
        {
            const bool firstUseThisFrame =
                configuredUnlitPrograms.insert(shaderProgramId).second;
            if (firstUseThisFrame)
            {
                material->shader->setMat4("projection", projection);
                material->shader->setMat4("view", view);
                material->shader->setFloat("uTime", sceneElapsedTime);
            }

            // Visualize each emitter with its configured light tint and
            // intensity.
            material->shader->setVec3("objectColor",
                                      runtimeObject.lightColor *
                                          runtimeObject.lightIntensity);
        }

        glBindVertexArray(object->getVAO());
        glm::mat4 model = object->getModelMatrix();
        material->shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0,
                     static_cast<GLsizei>(runtimeObject.vertexCount));
    }

    // Render scene text overlays
    if (textRenderer)
    {
        renderTextOverlay(overlayConfig, infoOverlayEnabled, fps,
                          sceneElapsedTime, currentTimeSeconds);
    }
}

void Scene::renderTextOverlay(const UIOverlayConfig& overlayConfig,
                              bool infoOverlayEnabled, float fps,
                              float sceneElapsedTime, float currentTimeSeconds)
{
    for (const TextDefinition& text : definition.texts)
    {
        textRenderer->renderText(text.text, text.x, text.y, text.scale,
                                 text.color);
    }

    // Render info overlay with FPS if enabled
    if (infoOverlayEnabled && overlayConfig.enabled)
    {
        glm::vec3 overlayColor(1.0f, 1.0f, 1.0f);
        float     yOffset = overlayConfig.y;

        // Dynamically render stats based on config
        for (const std::string& stat : overlayConfig.stats)
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
                int  totalMinutes = static_cast<int>(currentTimeSeconds) / 60;
                int  totalSeconds = static_cast<int>(currentTimeSeconds) % 60;
                char totalTimeBuffer[32];
                snprintf(totalTimeBuffer, sizeof(totalTimeBuffer),
                         "Total: %d:%02d", totalMinutes, totalSeconds);

                int  sceneMinutes = static_cast<int>(sceneElapsedTime) / 60;
                int  sceneSeconds = static_cast<int>(sceneElapsedTime) % 60;
                char sceneTimeBuffer[32];
                snprintf(sceneTimeBuffer, sizeof(sceneTimeBuffer),
                         "Scene: %d:%02d", sceneMinutes, sceneSeconds);

                statLine = std::string(totalTimeBuffer) + " | " +
                           std::string(sceneTimeBuffer);
            }
            else if (stat == "objects")
            {
                statLine = "Objects:";
                for (const SceneObjectDefinition& object : definition.objects)
                {
                    statLine += " " + object.id;
                }
            }
            else if (stat == "shaders")
            {
                statLine = "Shaders:";
                for (const MaterialDefinition& objectDef : definition.materials)
                {
                    statLine += " " + objectDef.id;
                }
            }

            if (!statLine.empty())
            {
                textRenderer->renderText(statLine, overlayConfig.x, yOffset,
                                         overlayConfig.scale, overlayColor);
                yOffset -= overlayConfig.lineSpacing;
            }
        }
    }
}
