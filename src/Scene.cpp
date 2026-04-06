#include "Scene.h"

#include <glad/glad.h>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <unordered_set>
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
    releaseShadowResources();
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
        runtimeObject.lightColor = objectDef.lightColor;
        runtimeObject.lightIntensity = objectDef.lightIntensity;

        runtimeObjects[objectDef.id] = runtimeObject;
    }

    // Resolve active light providers once for lit-object uniforms.
    activeLightSources.clear();
    for (auto &entry : runtimeObjects)
    {
        RuntimeSceneObject &runtimeObject = entry.second;
        if (runtimeObject.role == SceneRole::LightSource)
        {
            activeLightSources.push_back(&runtimeObject);
        }
    }

    if (activeLightSources.empty())
    {
        std::cout << "Scene definition must include an object with role: LightSource" << std::endl;
        return false;
    }

    if (definition.shadows.enabled && !initShadowResources())
    {
        std::cout << "Failed to initialize shadow resources" << std::endl;
        return false;
    }
    if (!definition.shadows.enabled)
    {
        releaseShadowResources();
    }

    shadowFrameCounter = 0;
    shadowCacheValid = false;

    return true;
}

void Scene::releaseShadowResources()
{
    if (shadowFramebuffer != 0)
    {
        glDeleteFramebuffers(1, &shadowFramebuffer);
        shadowFramebuffer = 0;
    }

    if (shadowDepthTexture != 0)
    {
        glDeleteTextures(1, &shadowDepthTexture);
        shadowDepthTexture = 0;
    }

    shadowDepthShader.reset();
}

bool Scene::initShadowResources()
{
    releaseShadowResources();

    shadowDepthShader = assets.getShader("assets/shaders/shadowDepth.vs", "assets/shaders/shadowDepth.fs", "assets/shaders/shadowDepth.gs");
    if (!shadowDepthShader)
    {
        return false;
    }

    glGenFramebuffers(1, &shadowFramebuffer);
    glGenTextures(1, &shadowDepthTexture);

    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowDepthTexture);
    for (int face = 0; face < 6; ++face)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                     0,
                     GL_DEPTH_COMPONENT,
                     definition.shadows.mapSize,
                     definition.shadows.mapSize,
                     0,
                     GL_DEPTH_COMPONENT,
                     GL_FLOAT,
                     nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowDepthTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    const bool isComplete = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return isComplete;
}

std::array<glm::mat4, 6> Scene::buildShadowCubeMatrices(const glm::vec3 &lightPosition) const
{
    const glm::mat4 shadowProjection = glm::perspective(glm::radians(definition.shadows.fovDegrees),
                                                        1.0f,
                                                        definition.shadows.nearPlane,
                                                        definition.shadows.farPlane);

    return {
        shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};
}

void Scene::renderShadowDepthPass(const std::array<glm::mat4, 6> &shadowMatrices, const glm::vec3 &lightPosition)
{
    glViewport(0, 0, definition.shadows.mapSize, definition.shadows.mapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Front-face culling in shadow pass reduces self-shadow acne artifacts.
    const GLboolean cullFaceWasEnabled = glIsEnabled(GL_CULL_FACE);
    if (!cullFaceWasEnabled)
    {
        glEnable(GL_CULL_FACE);
    }
    glCullFace(GL_FRONT);

    shadowDepthShader->use();
    for (std::size_t face = 0; face < shadowMatrices.size(); ++face)
    {
        shadowDepthShader->setMat4("shadowMatrices[" + std::to_string(face) + "]", shadowMatrices[face]);
    }
    shadowDepthShader->setVec3("lightPos", lightPosition);
    shadowDepthShader->setFloat("farPlane", definition.shadows.farPlane);

    for (const auto &entry : runtimeObjects)
    {
        const RuntimeSceneObject &runtimeObject = entry.second;
        if (runtimeObject.role == SceneRole::LightSource || runtimeObject.role == SceneRole::Ground)
        {
            continue;
        }

        shadowDepthShader->setMat4("model", runtimeObject.object->getModelMatrix());
        glBindVertexArray(runtimeObject.object->getVAO());
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(runtimeObject.vertexCount));
    }

    glCullFace(GL_BACK);
    if (!cullFaceWasEnabled)
    {
        glDisable(GL_CULL_FACE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    const int lightCount = std::min(static_cast<int>(activeLightSources.size()), MAX_LIGHT_SOURCES);
    std::array<glm::vec3, MAX_LIGHT_SOURCES> lightPositions{};
    std::array<glm::vec3, MAX_LIGHT_SOURCES> lightColors{};
    for (int i = 0; i < lightCount; ++i)
    {
        const RuntimeSceneObject *light = activeLightSources[static_cast<std::size_t>(i)];
        lightPositions[static_cast<std::size_t>(i)] = light->object->getPosition();
        lightColors[static_cast<std::size_t>(i)] = light->lightColor * light->lightIntensity;
    }

    if (definition.shadows.enabled && shadowFramebuffer != 0 && shadowDepthTexture != 0)
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        const glm::vec3 primaryLightPosition = lightPositions[0];
        const bool shouldUpdateShadowMap = !shadowCacheValid || (shadowFrameCounter++ % SHADOW_UPDATE_INTERVAL_FRAMES_DEFAULT == 0);
        if (shouldUpdateShadowMap)
        {
            renderShadowDepthPass(buildShadowCubeMatrices(primaryLightPosition), primaryLightPosition);
            shadowCacheValid = true;
        }

        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }

    if (definition.shadows.enabled && shadowDepthTexture != 0)
    {
        glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowDepthTexture);
    }

    static const std::array<std::string, MAX_LIGHT_SOURCES> lightPosUniformNames = []
    {
        std::array<std::string, MAX_LIGHT_SOURCES> names{};
        for (int i = 0; i < MAX_LIGHT_SOURCES; ++i)
        {
            names[static_cast<std::size_t>(i)] = "lightPos[" + std::to_string(i) + "]";
        }
        return names;
    }();

    static const std::array<std::string, MAX_LIGHT_SOURCES> lightColorUniformNames = []
    {
        std::array<std::string, MAX_LIGHT_SOURCES> names{};
        for (int i = 0; i < MAX_LIGHT_SOURCES; ++i)
        {
            names[static_cast<std::size_t>(i)] = "lightColor[" + std::to_string(i) + "]";
        }
        return names;
    }();

    std::unordered_set<unsigned int> configuredLitPrograms;
    std::unordered_set<unsigned int> configuredUnlitPrograms;

    // Render all objects with shared camera matrices and mode-specific uniforms.
    for (const auto &entry : runtimeObjects)
    {
        const RuntimeSceneObject &runtimeObject = entry.second;
        std::shared_ptr<Object> object = runtimeObject.object;

        std::shared_ptr<RuntimeMaterial> material = runtimeObject.material;
        material->shader->use();
        const unsigned int shaderProgramId = material->shader->ID;

        if (material->renderMode == RenderMode::Lit)
        {
            const bool firstUseThisFrame = configuredLitPrograms.insert(shaderProgramId).second;
            if (firstUseThisFrame)
            {
                material->shader->setMat4("projection", projection);
                material->shader->setMat4("view", view);
                material->shader->setFloat("uTime", sceneElapsedTime);
                material->shader->setVec3("viewPos", camera.Position);
                material->shader->setInt("lightCount", lightCount);
                material->shader->setInt("shadowEnabled", definition.shadows.enabled ? 1 : 0);
                material->shader->setFloat("shadowBiasMin", definition.shadows.biasMin);
                material->shader->setFloat("shadowBiasSlope", definition.shadows.biasSlope);
                material->shader->setFloat("shadowFarPlane", definition.shadows.farPlane);
                material->shader->setInt("shadowMap", SHADOW_MAP_TEXTURE_UNIT);

                if (lightCount > 0)
                {
                    // Backward-compatible single-light uniforms for legacy shaders.
                    material->shader->setVec3("lightColor", lightColors[0]);
                    material->shader->setVec3("lightPos", lightPositions[0]);
                }

                for (int i = 0; i < lightCount; ++i)
                {
                    const std::size_t idx = static_cast<std::size_t>(i);
                    material->shader->setVec3(lightPosUniformNames[idx], lightPositions[idx]);
                    material->shader->setVec3(lightColorUniformNames[idx], lightColors[idx]);
                }
            }

            material->shader->setVec3("objectColor", material->objectColor);
        }
        else if (material->renderMode == RenderMode::LightSource)
        {
            const bool firstUseThisFrame = configuredUnlitPrograms.insert(shaderProgramId).second;
            if (firstUseThisFrame)
            {
                material->shader->setMat4("projection", projection);
                material->shader->setMat4("view", view);
                material->shader->setFloat("uTime", sceneElapsedTime);
            }

            // Visualize each emitter with its configured light tint and intensity.
            material->shader->setVec3("objectColor", runtimeObject.lightColor * runtimeObject.lightIntensity);
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