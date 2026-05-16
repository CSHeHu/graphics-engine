#include "Scene.h"

#include <algorithm>
#include <array>
#include <glm.hpp>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <utility>

#include "AssetManager.h"
#include "AudioManager.h"
#include "Camera.h"
#include "GpuMesh.h"
#include "InstanceBuffer.h"
#include "Object.h"
#include "SceneDefinition.h"
#include "SceneOverlayRenderer.h"
#include "Shader.h"
#include "TextManager.h"

Scene::Scene(AssetManager& assetManager, SceneDefinition& definitionValue,
             TextManager&           textManager,
             const RenderingConfig& renderingConfigValue,
             AudioManager&          audioManager)
    : assets(assetManager), textRenderer(textManager), audio(audioManager),
      definition(definitionValue), renderingConfig(renderingConfigValue),
      overlayRenderer(std::make_unique<SceneOverlayRenderer>())
{
}

Scene::~Scene() = default;

bool Scene::init()
{
    runtimeMaterials.clear();
    instanceBuffers.clear();
    runtimeObjects.clear();
    runtimeObjectOrder.clear();

    if (!initializeRuntimeMaterials())
    {
        return false;
    }

    if (!initializeRuntimeObjects())
    {
        return false;
    }

    collectActiveLightSources();

    if (activeLightSources.empty())
    {
        std::cout
            << "Scene definition must include an object with role: LightSource"
            << std::endl;
        return false;
    }

    if (definition.audio.continueOnSceneChange)
    {
        // Keep the current track alive and playing across scene switches.
    }
    else if (definition.audio.musicPath.empty())
    {
        audio.stop();
    }
    else
    {
        std::shared_ptr<Mix_Music> sceneMusic =
            assets.loadAudio(definition.audio.musicPath);
        if (audio.play(sceneMusic, definition.audio.loops) != 0)
        {
            std::cout << "Failed to start scene music: "
                      << definition.audio.musicPath << std::endl;
            return false;
        }
    }

    return true;
}

bool Scene::initializeRuntimeMaterials()
{
    for (const MaterialDefinition& materialDef : definition.materials)
    {
        std::shared_ptr<RuntimeMaterial> material =
            std::make_shared<RuntimeMaterial>();
        material->shader = assets.loadShader(materialDef.vertexShaderPath,
                                             materialDef.fragmentShaderPath,
                                             materialDef.geometryShaderPath);
        material->renderMode  = materialDef.renderMode;
        material->objectColor = materialDef.objectColor;

        runtimeMaterials[materialDef.id] = material;
    }

    return true;
}

bool Scene::initializeRuntimeObjects()
{
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

        const std::shared_ptr<GpuMesh> gpuMesh =
            assets.loadGpuMesh(objectDef.meshName, objectDef.layout);
        std::shared_ptr<RuntimeMaterial> material = materialIt->second;

        const std::string instanceKey =
            objectDef.meshName + ":" + objectDef.materialId;

        // Get or create InstanceBuffer for this mesh/material pair.
        auto instanceBufferIt = instanceBuffers.find(instanceKey);
        if (instanceBufferIt == instanceBuffers.end())
        {
            instanceBuffers[instanceKey] = std::make_shared<InstanceBuffer>();
            instanceBufferIt             = instanceBuffers.find(instanceKey);
        }
        std::shared_ptr<InstanceBuffer> instanceBuffer =
            instanceBufferIt->second;

        std::shared_ptr<Object> object =
            std::make_shared<Object>(objectDef.position, objectDef.scale);

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

        InstanceBuffer::InstanceData instanceData;
        instanceData.basePosition      = objectDef.position;
        instanceData.baseRotationAngle = object->getRotationAngle();
        instanceData.baseScale         = objectDef.scale;
        instanceData.behaviorType =
            static_cast<float>(static_cast<int>(objectDef.behavior));
        instanceData.baseRotationAxis  = object->getRotationAxis();
        instanceData.behaviorSpeed     = objectDef.behaviorSpeed;
        instanceData.behaviorAxis      = objectDef.behaviorAxis;
        instanceData.behaviorAmplitude = objectDef.behaviorAmplitude;
        instanceData.instanceColor =
            objectDef.role == SceneRole::LightSource
                ? glm::vec4(objectDef.lightColor * objectDef.lightIntensity,
                            1.0f)
                : glm::vec4(1.0f);
        const std::size_t instanceIndex =
            instanceBuffer->addInstance(instanceData);

        auto runtimeObject             = std::make_shared<RuntimeSceneObject>();
        runtimeObject->core.mesh       = gpuMesh;
        runtimeObject->core.object     = object;
        runtimeObject->core.material   = material;
        runtimeObject->core.role       = objectDef.role;
        runtimeObject->core.meshName   = objectDef.meshName;
        runtimeObject->core.materialId = objectDef.materialId;
        runtimeObject->core.instanceBuffer = instanceBuffer;
        runtimeObject->core.instanceIndex  = instanceIndex;
        runtimeObject->core.cullingRadius =
            std::max({objectDef.scale.x, objectDef.scale.y, objectDef.scale.z});

        runtimeObject->behavior.type            = objectDef.behavior;
        runtimeObject->behavior.oscillate.speed = objectDef.behaviorSpeed;
        runtimeObject->behavior.oscillate.axis  = objectDef.behaviorAxis;
        runtimeObject->behavior.oscillate.amplitude =
            objectDef.behaviorAmplitude;
        runtimeObject->behavior.oscillate.initialPosition = objectDef.position;

        runtimeObject->behavior.spin.speed = objectDef.behaviorSpeed;
        runtimeObject->behavior.spin.axis  = objectDef.behaviorAxis;
        runtimeObject->behavior.spin.initialRotationAngle =
            object->getRotationAngle();

        runtimeObject->behavior.fly.speed           = objectDef.behaviorSpeed;
        runtimeObject->behavior.fly.direction       = objectDef.behaviorAxis;
        runtimeObject->behavior.fly.initialPosition = objectDef.position;

        runtimeObject->light.color     = objectDef.lightColor;
        runtimeObject->light.intensity = objectDef.lightIntensity;

        runtimeObjects[objectDef.id] = runtimeObject;
        runtimeObjectOrder.push_back(objectDef.id);
    }

    return true;
}

std::array<Scene::FrustumPlane, 6>
Scene::buildFrustumPlanes(const glm::mat4& projection,
                          const glm::mat4& view) const
{
    const glm::mat4 clip = projection * view;

    auto makePlane = [](float a, float b, float c, float d)
    {
        const glm::vec3 normal(a, b, c);
        const float     lengthNormal = glm::length(normal);
        if (lengthNormal <= 0.0001f)
        {
            return FrustumPlane{glm::vec3(0.0f, 0.0f, 1.0f), 0.0f};
        }
        return FrustumPlane{normal / lengthNormal, d / lengthNormal};
    };

    std::array<FrustumPlane, 6> planes;
    planes[0] = makePlane(clip[0][3] + clip[0][0], clip[1][3] + clip[1][0],
                          clip[2][3] + clip[2][0], clip[3][3] + clip[3][0]);
    planes[1] = makePlane(clip[0][3] - clip[0][0], clip[1][3] - clip[1][0],
                          clip[2][3] - clip[2][0], clip[3][3] - clip[3][0]);
    planes[2] = makePlane(clip[0][3] + clip[0][1], clip[1][3] + clip[1][1],
                          clip[2][3] + clip[2][1], clip[3][3] + clip[3][1]);
    planes[3] = makePlane(clip[0][3] - clip[0][1], clip[1][3] - clip[1][1],
                          clip[2][3] - clip[2][1], clip[3][3] - clip[3][1]);
    planes[4] = makePlane(clip[0][3] + clip[0][2], clip[1][3] + clip[1][2],
                          clip[2][3] + clip[2][2], clip[3][3] + clip[3][2]);
    planes[5] = makePlane(clip[0][3] - clip[0][2], clip[1][3] - clip[1][2],
                          clip[2][3] - clip[2][2], clip[3][3] - clip[3][2]);
    return planes;
}

bool Scene::isRuntimeObjectVisible(
    const RuntimeSceneObject&          runtimeObject,
    const std::array<FrustumPlane, 6>& frustumPlanes) const
{
    // Fly behavior drifts infinitely over time in shaders, so static CPU-side
    // bounds cannot be conservative without becoming useless.
    if (runtimeObject.behavior.type == BehaviorType::Fly)
    {
        return true;
    }

    const glm::vec3 center = runtimeObject.core.object->getPosition();
    float           radius = std::max(runtimeObject.core.cullingRadius, 0.1f);

    if (runtimeObject.behavior.type == BehaviorType::Oscillate)
    {
        radius += std::max(runtimeObject.behavior.oscillate.amplitude, 0.0f);
    }

    for (const FrustumPlane& plane : frustumPlanes)
    {
        const float signedDistance =
            glm::dot(plane.normal, center) + plane.distance;
        if (signedDistance < -radius)
        {
            return false;
        }
    }

    return true;
}

void Scene::collectActiveLightSources()
{
    // Resolve active light providers once for lit-object uniforms.
    activeLightSources.clear();
    for (const std::string& objectId : runtimeObjectOrder)
    {
        std::shared_ptr<RuntimeSceneObject> runtimeObject =
            runtimeObjects.at(objectId);
        if (runtimeObject->core.role == SceneRole::LightSource)
        {
            activeLightSources.push_back(runtimeObject);
        }
    }
}

Scene::PerFrameLightUniforms
Scene::collectLightUniforms(int maxLightSources) const
{
    PerFrameLightUniforms perFrameLightUniforms;
    perFrameLightUniforms.lightCount =
        std::min(static_cast<int>(activeLightSources.size()), maxLightSources);
    perFrameLightUniforms.lightBasePositions.reserve(
        static_cast<std::size_t>(perFrameLightUniforms.lightCount));
    perFrameLightUniforms.lightBehaviorTypes.reserve(
        static_cast<std::size_t>(perFrameLightUniforms.lightCount));
    perFrameLightUniforms.lightBehaviorSpeeds.reserve(
        static_cast<std::size_t>(perFrameLightUniforms.lightCount));
    perFrameLightUniforms.lightBehaviorAxes.reserve(
        static_cast<std::size_t>(perFrameLightUniforms.lightCount));
    perFrameLightUniforms.lightBehaviorAmplitudes.reserve(
        static_cast<std::size_t>(perFrameLightUniforms.lightCount));
    perFrameLightUniforms.lightColors.reserve(
        static_cast<std::size_t>(perFrameLightUniforms.lightCount));
    for (int i = 0; i < perFrameLightUniforms.lightCount; ++i)
    {
        const std::shared_ptr<RuntimeSceneObject> light =
            activeLightSources[static_cast<std::size_t>(i)];
        perFrameLightUniforms.lightBasePositions.push_back(
            light->core.object->getPosition());
        perFrameLightUniforms.lightBehaviorTypes.push_back(
            static_cast<int>(light->behavior.type));
        const float behaviorSpeed = light->behavior.type == BehaviorType::Spin
                                        ? light->behavior.spin.speed
                                    : light->behavior.type == BehaviorType::Fly
                                        ? light->behavior.fly.speed
                                        : light->behavior.oscillate.speed;
        const glm::vec3 behaviorAxis =
            light->behavior.type == BehaviorType::Spin
                ? light->behavior.spin.axis
            : light->behavior.type == BehaviorType::Fly
                ? light->behavior.fly.direction
                : light->behavior.oscillate.axis;
        const float behaviorAmplitude = light->behavior.oscillate.amplitude;
        perFrameLightUniforms.lightBehaviorSpeeds.push_back(behaviorSpeed);
        perFrameLightUniforms.lightBehaviorAxes.push_back(behaviorAxis);
        perFrameLightUniforms.lightBehaviorAmplitudes.push_back(
            behaviorAmplitude);
        perFrameLightUniforms.lightColors.push_back(light->light.color *
                                                    light->light.intensity);
    }

    return perFrameLightUniforms;
}

void Scene::renderRuntimeObjects(
    const Camera& camera, const glm::mat4& projection, const glm::mat4& view,
    float sceneElapsedTime, const PerFrameLightUniforms& perFrameLightUniforms)
{
    std::unordered_set<unsigned int>  configuredLitPrograms;
    std::unordered_set<unsigned int>  configuredUnlitPrograms;
    const std::array<FrustumPlane, 6> frustumPlanes =
        buildFrustumPlanes(projection, view);

    // Group objects by mesh/material so the mesh VAO and instance buffer can
    // be reused across matching objects.
    struct MeshMaterialGroup
    {
            std::string              meshName;
            std::string              materialId;
            std::vector<std::string> objectIds;
    };
    std::unordered_map<std::string, MeshMaterialGroup> groups;

    for (const std::string& objectId : runtimeObjectOrder)
    {
        const std::shared_ptr<RuntimeSceneObject> runtimeObject =
            runtimeObjects.at(objectId);
        const std::string groupKey =
            runtimeObject->core.meshName + ":" + runtimeObject->core.materialId;

        if (groups.find(groupKey) == groups.end())
        {
            groups[groupKey] = {runtimeObject->core.meshName,
                                runtimeObject->core.materialId,
                                {}};
        }
        groups[groupKey].objectIds.push_back(objectId);
    }

    // Render groups with shared mesh/material state.
    for (auto& [groupKey, group] : groups)
    {
        (void)groupKey;
        if (group.objectIds.empty())
            continue;

        // Fetch first object to get mesh and material references
        const std::shared_ptr<RuntimeSceneObject> firstObject =
            runtimeObjects.at(group.objectIds.front());
        std::shared_ptr<GpuMesh>         mesh     = firstObject->core.mesh;
        std::shared_ptr<RuntimeMaterial> material = firstObject->core.material;

        mesh->bind();

        material->shader->use();
        const unsigned int shaderProgramId = material->shader->ID;

        if (material->renderMode == RenderMode::Lit)
        {
            const bool firstUseThisFrame =
                configuredLitPrograms.insert(shaderProgramId).second;
            if (firstUseThisFrame)
            {
                configureLitShaderUniforms(material, camera, projection, view,
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
                configureEmitterShaderUniforms(material, projection, view,
                                               sceneElapsedTime);
            }
        }

        if (!firstObject->core.instanceBuffer)
        {
            continue;
        }

        std::vector<std::size_t> visibleInstanceIndices;
        visibleInstanceIndices.reserve(group.objectIds.size());
        for (const std::string& objectId : group.objectIds)
        {
            const std::shared_ptr<RuntimeSceneObject> runtimeObject =
                runtimeObjects.at(objectId);
            if (isRuntimeObjectVisible(*runtimeObject, frustumPlanes))
            {
                visibleInstanceIndices.push_back(
                    runtimeObject->core.instanceIndex);
            }
        }

        if (visibleInstanceIndices.empty())
        {
            continue;
        }

        firstObject->core.instanceBuffer->setDrawInstances(
            visibleInstanceIndices);
        firstObject->core.instanceBuffer->attachToBoundVao();

        const std::size_t instanceCount = visibleInstanceIndices.size();
        glDrawElementsInstanced(
            GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndexCount()),
            GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(instanceCount));
    }
}

void Scene::configureLitShaderUniforms(
    const std::shared_ptr<RuntimeMaterial>& material, const Camera& camera,
    const glm::mat4& projection, const glm::mat4& view, float sceneElapsedTime,
    const PerFrameLightUniforms& perFrameLightUniforms) const
{
    // These values are frame-wide, so upload them once per shader program
    // rather than once per object.
    material->shader->setMat4("projection", projection);
    material->shader->setMat4("view", view);
    material->shader->setFloat("uTime", sceneElapsedTime);
    material->shader->setVec3("viewPos", camera.getPosition());
    material->shader->setInt("lightCount", perFrameLightUniforms.lightCount);

    for (int i = 0; i < perFrameLightUniforms.lightCount; ++i)
    {
        const std::size_t idx = static_cast<std::size_t>(i);
        const std::string basePosUniformName =
            "lightBasePos[" + std::to_string(i) + "]";
        const std::string behaviorTypeUniformName =
            "lightBehaviorType[" + std::to_string(i) + "]";
        const std::string behaviorSpeedUniformName =
            "lightBehaviorSpeed[" + std::to_string(i) + "]";
        const std::string behaviorAxisUniformName =
            "lightBehaviorAxis[" + std::to_string(i) + "]";
        const std::string behaviorAmplitudeUniformName =
            "lightBehaviorAmplitude[" + std::to_string(i) + "]";
        const std::string colorUniformName =
            "lightColor[" + std::to_string(i) + "]";
        material->shader->setVec3(
            basePosUniformName, perFrameLightUniforms.lightBasePositions[idx]);
        material->shader->setInt(behaviorTypeUniformName,
                                 perFrameLightUniforms.lightBehaviorTypes[idx]);
        material->shader->setFloat(
            behaviorSpeedUniformName,
            perFrameLightUniforms.lightBehaviorSpeeds[idx]);
        material->shader->setVec3(behaviorAxisUniformName,
                                  perFrameLightUniforms.lightBehaviorAxes[idx]);
        material->shader->setFloat(
            behaviorAmplitudeUniformName,
            perFrameLightUniforms.lightBehaviorAmplitudes[idx]);
        material->shader->setVec3(colorUniformName,
                                  perFrameLightUniforms.lightColors[idx]);
    }
}

void Scene::configureEmitterShaderUniforms(
    const std::shared_ptr<RuntimeMaterial>& material,
    const glm::mat4& projection, const glm::mat4& view,
    float sceneElapsedTime) const
{
    // Unlit objects still share the camera matrices and time.
    material->shader->setMat4("projection", projection);
    material->shader->setMat4("view", view);
    material->shader->setFloat("uTime", sceneElapsedTime);
}

void Scene::render(const Camera& camera, const glm::mat4& projection,
                   const glm::mat4& view, float fps, float sceneElapsedTime,
                   const UIOverlayConfig& overlayConfig,
                   bool infoOverlayEnabled, float currentTimeSeconds)
{
    // Runtime rendering limits and bindings are centralized in
    // scene_config.json.
    const int maxLightSources = std::max(renderingConfig.maxLightSources, 1);

    const PerFrameLightUniforms perFrameLightUniforms =
        collectLightUniforms(maxLightSources);

    // Draw visible scene objects.
    renderRuntimeObjects(camera, projection, view, sceneElapsedTime,
                         perFrameLightUniforms);

    // Render scene text and runtime overlays through a dedicated presenter.
    overlayRenderer->render(textRenderer, definition, overlayConfig,
                            infoOverlayEnabled, fps, sceneElapsedTime,
                            currentTimeSeconds);
}
