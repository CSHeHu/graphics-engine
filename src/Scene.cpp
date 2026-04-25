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
#include "TextManager.h"

Scene::Scene(AssetManager&                    assetManager,
             std::shared_ptr<SceneDefinition> definitionValue,
             TextManager&                     textManager)
    : assets(assetManager), textRenderer(textManager),
    definition(std::move(definitionValue)),
    lightUniformNameTable{0, {}, {}},
    behaviorHandlers{&Scene::applyBehaviorNone,
                 &Scene::applyBehaviorOscillate,
                 &Scene::applyBehaviorSpin,
                 &Scene::applyBehaviorFly}
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

    
    if (!initializeRuntimeMaterials())
    {
        return false;
    }

    if (!initializeRuntimeObjects())
    {
        return false;
    }

    refreshActiveLightSources();

    if (activeLightSources.empty())
    {
        std::cout
            << "Scene definition must include an object with role: LightSource"
            << std::endl;
        return false;
    }

    return true;
}

bool Scene::initializeRuntimeMaterials()
{
    for (const MaterialDefinition& materialDef : definition->materials)
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

    return true;
}

bool Scene::initializeRuntimeObjects()
{
    // Build runtime objects from scene definition data.
    for (const SceneObjectDefinition& objectDef : definition->objects)
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
        const std::vector<unsigned int>& indices =
            assets.getMeshIndices(objectDef.meshName);
        const std::size_t indexCount =
            assets.getMeshIndexCount(objectDef.meshName);
        std::shared_ptr<RuntimeMaterial> material = materialIt->second;

        std::shared_ptr<Object> object = std::make_shared<Object>(
            material->shader, vertices, indices,
            objectDef.position, objectDef.scale,
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
        runtimeObject.indexCount           = indexCount;
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

    return true;
}

void Scene::refreshActiveLightSources()
{
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
}

Scene::PerFrameLightUniforms
Scene::buildPerFrameLightUniforms(int maxLightSources) const
{
    PerFrameLightUniforms perFrameLightUniforms;
    perFrameLightUniforms.lightCount =
        std::min(static_cast<int>(activeLightSources.size()), maxLightSources);
    perFrameLightUniforms.lightPositions.assign(
        static_cast<std::size_t>(maxLightSources), glm::vec3(0.0f));
    perFrameLightUniforms.lightColors.assign(
        static_cast<std::size_t>(maxLightSources), glm::vec3(0.0f));
    for (int i = 0; i < perFrameLightUniforms.lightCount; ++i)
    {
        const RuntimeSceneObject* light =
            activeLightSources[static_cast<std::size_t>(i)];
        perFrameLightUniforms.lightPositions[static_cast<std::size_t>(i)] =
            light->object->getPosition();
        perFrameLightUniforms.lightColors[static_cast<std::size_t>(i)] =
            light->lightColor * light->lightIntensity;
    }

    return perFrameLightUniforms;
}

void Scene::ensureLightUniformNameTable(int maxLightSources)
{
    if (lightUniformNameTable.capacity == maxLightSources)
    {
        return;
    }

    lightUniformNameTable.lightPosNames.resize(
        static_cast<std::size_t>(maxLightSources));
    lightUniformNameTable.lightColorNames.resize(
        static_cast<std::size_t>(maxLightSources));
    for (int i = 0; i < maxLightSources; ++i)
    {
        const std::size_t idx = static_cast<std::size_t>(i);
        lightUniformNameTable.lightPosNames[idx] =
            "lightPos[" + std::to_string(i) + "]";
        lightUniformNameTable.lightColorNames[idx] =
            "lightColor[" + std::to_string(i) + "]";
    }
    lightUniformNameTable.capacity = maxLightSources;
}

void Scene::renderRuntimeObjects(const Camera&    camera,
                                 const glm::mat4& projection,
                                 const glm::mat4& view, float sceneElapsedTime,
                                 int maxLightSources,
                                 const PerFrameLightUniforms& perFrameLightUniforms)
{
    ensureLightUniformNameTable(maxLightSources);

    std::unordered_set<unsigned int> configuredLitPrograms;
    std::unordered_set<unsigned int> configuredUnlitPrograms;

    // Render all objects with shared camera matrices and mode-specific
    // uniforms.
    for (const auto& entry : runtimeObjects)
    {
        const RuntimeSceneObject& runtimeObject = entry.second;

        std::shared_ptr<RuntimeMaterial> material = runtimeObject.material;
        material->shader->use();
        const unsigned int shaderProgramId = material->shader->ID;

        if (material->renderMode == RenderMode::Lit)
        {
            const bool firstUseThisFrame =
                configuredLitPrograms.insert(shaderProgramId).second;
            if (firstUseThisFrame)
            {
                configureLitShaderPerFrame(material, camera, projection, view,
                                           sceneElapsedTime,
                                           perFrameLightUniforms);
            }

            material->shader->setVec3("objectColor", material->objectColor);
        }
        else if (material->renderMode == RenderMode::LightSource)
        {
            const bool firstUseThisFrame =
                configuredUnlitPrograms.insert(shaderProgramId).second;
            if (firstUseThisFrame)
            {
                configureLightSourceShaderPerFrame(material, projection, view,
                                                   sceneElapsedTime);
            }

            // Visualize each emitter with its configured light tint and
            // intensity.
            material->shader->setVec3("objectColor",
                                      runtimeObject.lightColor *
                                          runtimeObject.lightIntensity);
        }

        drawRuntimeObject(runtimeObject);
    }
}

void Scene::configureLitShaderPerFrame(
    const std::shared_ptr<RuntimeMaterial>& material,
    const Camera& camera, const glm::mat4& projection,
    const glm::mat4& view, float sceneElapsedTime,
    const PerFrameLightUniforms& perFrameLightUniforms) const
{
    // These values are frame-wide, so upload them once per shader program
    // rather than once per object.
    material->shader->setMat4("projection", projection);
    material->shader->setMat4("view", view);
    material->shader->setFloat("uTime", sceneElapsedTime);
    material->shader->setVec3("viewPos", camera.getPosition());
    material->shader->setInt("lightCount", perFrameLightUniforms.lightCount);

    if (perFrameLightUniforms.lightCount > 0)
    {
        // Backward-compatible single-light uniforms for legacy shaders.
        material->shader->setVec3("lightColor",
                                  perFrameLightUniforms.lightColors[0]);
        material->shader->setVec3("lightPos",
                                  perFrameLightUniforms.lightPositions[0]);
    }

    for (int i = 0; i < perFrameLightUniforms.lightCount; ++i)
    {
        const std::size_t idx = static_cast<std::size_t>(i);
        material->shader->setVec3(lightUniformNameTable.lightPosNames[idx],
                                  perFrameLightUniforms.lightPositions[idx]);
        material->shader->setVec3(lightUniformNameTable.lightColorNames[idx],
                                  perFrameLightUniforms.lightColors[idx]);
    }
}

void Scene::configureLightSourceShaderPerFrame(
    const std::shared_ptr<RuntimeMaterial>& material,
    const glm::mat4& projection, const glm::mat4& view,
    float sceneElapsedTime) const
{
    // Unlit objects still share the camera matrices and time.
    material->shader->setMat4("projection", projection);
    material->shader->setMat4("view", view);
    material->shader->setFloat("uTime", sceneElapsedTime);
}

void Scene::update(float sceneElapsedTime)
{
    // Behaviors are evaluated from the spawn transform, which keeps them
    // deterministic and avoids accumulating floating-point drift.
    for (auto& entry : runtimeObjects)
    {
        updateRuntimeObjectBehavior(entry.second, sceneElapsedTime);
    }
}

void Scene::updateRuntimeObjectBehavior(RuntimeSceneObject& runtimeObject,
                                        float               sceneElapsedTime)
{
    std::size_t behaviorIndex = static_cast<std::size_t>(runtimeObject.behavior);
    if (behaviorIndex >= behaviorHandlers.size())
    {
        behaviorIndex = 0;
    }

    const BehaviorHandler handler = behaviorHandlers[behaviorIndex];
    (this->*handler)(runtimeObject, sceneElapsedTime);
}

void Scene::applyBehaviorNone(RuntimeSceneObject& runtimeObject,
                              float               sceneElapsedTime)
{
    (void)runtimeObject;
    (void)sceneElapsedTime;
}

void Scene::applyBehaviorOscillate(RuntimeSceneObject& runtimeObject,
                                   float               sceneElapsedTime)
{
    std::shared_ptr<Object> object = runtimeObject.object;
    const float             delta =
        std::sin(sceneElapsedTime * runtimeObject.behaviorSpeed) *
        runtimeObject.behaviorAmplitude;
    object->setPosition(runtimeObject.initialPosition +
                        runtimeObject.behaviorAxis * delta);
}

void Scene::applyBehaviorSpin(RuntimeSceneObject& runtimeObject,
                              float               sceneElapsedTime)
{
    std::shared_ptr<Object> object = runtimeObject.object;
    object->setRotation(runtimeObject.initialRotationAngle +
                            runtimeObject.behaviorSpeed * sceneElapsedTime,
                        runtimeObject.behaviorAxis);
}

void Scene::applyBehaviorFly(RuntimeSceneObject& runtimeObject,
                             float               sceneElapsedTime)
{
    std::shared_ptr<Object> object    = runtimeObject.object;
    const glm::vec3         direction =
        glm::normalize(runtimeObject.behaviorAxis);
    const glm::vec3 displacement =
        direction * (runtimeObject.behaviorSpeed * sceneElapsedTime);
    object->setPosition(runtimeObject.initialPosition + displacement);
}

void Scene::drawRuntimeObject(const RuntimeSceneObject& runtimeObject) const
{
    std::shared_ptr<Object> object   = runtimeObject.object;
    std::shared_ptr<RuntimeMaterial> material = runtimeObject.material;
    glBindVertexArray(object->getVAO());
    glm::mat4 model = object->getModelMatrix();
    material->shader->setMat4("model", model);
    const glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    material->shader->setMat3("normalMatrix", normalMatrix);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(runtimeObject.indexCount),
                   GL_UNSIGNED_INT, nullptr);
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

    const PerFrameLightUniforms perFrameLightUniforms =
        buildPerFrameLightUniforms(maxLightSources);

    // Draw visible scene objects.
    renderRuntimeObjects(camera, projection, view, sceneElapsedTime,
                         maxLightSources, perFrameLightUniforms);

    // Render scene text overlays
    renderTextOverlay(overlayConfig, infoOverlayEnabled, fps, sceneElapsedTime,
                      currentTimeSeconds);
}

void Scene::renderTextOverlay(const UIOverlayConfig& overlayConfig,
                              bool infoOverlayEnabled, float fps,
                              float sceneElapsedTime, float currentTimeSeconds)
{
    for (const TextDefinition& text : definition->texts)
    {
        textRenderer.renderText(text.text, text.x, text.y, text.scale,
                                text.color);
    }

    // The overlay is config-driven: each stat token picks a different line.
    if (infoOverlayEnabled && overlayConfig.enabled)
    {
        glm::vec3 overlayColor(1.0f, 1.0f, 1.0f);
        float     yOffset = overlayConfig.y;

        for (const std::string& stat : overlayConfig.stats)
        {
            std::string statLine;

            if (stat == "fps")
            {
                statLine = "FPS: " + std::to_string(static_cast<int>(fps));
            }
            else if (stat == "sceneName")
            {
                statLine = "Scene: " + definition->name;
            }
            else if (stat == "time")
            {
                // Show both global time and current-scene time to make
                // timeline changes obvious during playback.
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
                for (const SceneObjectDefinition& object : definition->objects)
                {
                    statLine += " " + object.id;
                }
            }
            else if (stat == "shaders")
            {
                statLine = "Shaders:";
                for (const MaterialDefinition& objectDef :
                     definition->materials)
                {
                    statLine += " " + objectDef.id;
                }
            }

            if (!statLine.empty())
            {
                textRenderer.renderText(statLine, overlayConfig.x, yOffset,
                                        overlayConfig.scale, overlayColor);
                yOffset -= overlayConfig.lineSpacing;
            }
        }
    }
}
