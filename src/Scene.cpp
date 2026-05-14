#include "Scene.h"

#include <algorithm>
#include <cmath>
#include <glm.hpp>
#include <iostream>
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

Scene::Scene(AssetManager&                    assetManager,
             std::shared_ptr<SceneDefinition> definitionValue,
             TextManager&                     textManager,
             const RenderingConfig&           renderingConfigValue,
             AudioManager&                    audioManager)
    : assets(assetManager), textRenderer(textManager), audio(audioManager),
      definition(std::move(definitionValue)),
      renderingConfig(renderingConfigValue), lightUniformNameTable{0, {}, {}},
      overlayRenderer(std::make_unique<SceneOverlayRenderer>())
{
}

Scene::~Scene() = default;

std::array<Scene::FrustumPlane, Scene::kFrustumPlaneCount>
Scene::buildFrustumPlanes(const glm::mat4& viewProjection) const
{
    std::array<FrustumPlane, kFrustumPlaneCount> planes;

    const auto getColumn = [&viewProjection](MatrixColumn column)
    {
        const std::size_t columnIndex = static_cast<std::size_t>(column);
        return glm::vec4(
            viewProjection[columnIndex][0], viewProjection[columnIndex][1],
            viewProjection[columnIndex][2], viewProjection[columnIndex][3]);
    };

    const glm::vec4 column0 = getColumn(MatrixColumn::X);
    const glm::vec4 column1 = getColumn(MatrixColumn::Y);
    const glm::vec4 column2 = getColumn(MatrixColumn::Z);
    const glm::vec4 column3 = getColumn(MatrixColumn::W);

    const glm::vec4 rawPlanes[Scene::kFrustumPlaneCount] = {
        column3 + column0, column3 - column0, column3 + column1,
        column3 - column1, column3 + column2, column3 - column2};

    for (std::size_t i = 0; i < planes.size(); ++i)
    {
        const glm::vec4 plane         = rawPlanes[i];
        const float     length        = glm::length(glm::vec3(plane));
        const float     inverseLength = length > 0.0f ? 1.0f / length : 1.0f;
        planes[i].normal              = glm::vec3(plane) * inverseLength;
        planes[i].distance            = plane.w * inverseLength;
    }

    return planes;
}

bool Scene::isRuntimeObjectVisible(
    const RuntimeSceneObject&          runtimeObject,
    const std::array<FrustumPlane, 6>& frustumPlanes) const
{
    const glm::mat4 modelMatrix = runtimeObject.core.object->getModelMatrix();
    const glm::vec3 center(
        modelMatrix[static_cast<std::size_t>(MatrixColumn::W)]);
    const float radius = computeBoundingRadius(*runtimeObject.core.object);

    for (const FrustumPlane& plane : frustumPlanes)
    {
        if (glm::dot(plane.normal, center) + plane.distance < -radius)
        {
            return false;
        }
    }

    return true;
}

float Scene::computeBoundingRadius(const Object& object) const
{
    const glm::mat4 modelMatrix = object.getModelMatrix();
    const float     scaleX      = glm::length(
        glm::vec3(modelMatrix[static_cast<std::size_t>(MatrixColumn::X)]));
    const float scaleY = glm::length(
        glm::vec3(modelMatrix[static_cast<std::size_t>(MatrixColumn::Y)]));
    const float scaleZ = glm::length(
        glm::vec3(modelMatrix[static_cast<std::size_t>(MatrixColumn::Z)]));
    constexpr float kBoundingRadiusPadding = 1.5f;
    return std::max(std::max(scaleX, scaleY), scaleZ) * kBoundingRadiusPadding;
}

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

    if (definition->audio.continueOnSceneChange)
    {
        // Keep the current track alive and playing across scene switches.
    }
    else if (definition->audio.musicPath.empty())
    {
        audio.stop();
    }
    else
    {
        std::shared_ptr<Mix_Music> sceneMusic =
            assets.loadAudio(definition->audio.musicPath);
        if (audio.play(sceneMusic, definition->audio.loops) != 0)
        {
            std::cout << "Failed to start scene music: "
                      << definition->audio.musicPath << std::endl;
            return false;
        }
    }

    return true;
}

bool Scene::initializeRuntimeMaterials()
{
    for (const MaterialDefinition& materialDef : definition->materials)
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

        // Add instance to buffer and track index
        const glm::vec4 instanceColor =
            objectDef.role == SceneRole::LightSource
                ? glm::vec4(objectDef.lightColor * objectDef.lightIntensity,
                            1.0f)
                : glm::vec4(1.0f);
        glm::mat4   modelMatrix = object->getModelMatrix();
        std::size_t instanceIndex =
            instanceBuffer->addInstance(modelMatrix, instanceColor);

        RuntimeSceneObject runtimeObject;
        runtimeObject.core.mesh           = gpuMesh;
        runtimeObject.core.object         = object;
        runtimeObject.core.material       = material;
        runtimeObject.core.role           = objectDef.role;
        runtimeObject.core.meshName       = objectDef.meshName;
        runtimeObject.core.materialId     = objectDef.materialId;
        runtimeObject.core.instanceIndex  = instanceIndex;
        runtimeObject.core.instanceBuffer = instanceBuffer;

        runtimeObject.behavior.type            = objectDef.behavior;
        runtimeObject.behavior.oscillate.speed = objectDef.behaviorSpeed;
        runtimeObject.behavior.oscillate.axis  = objectDef.behaviorAxis;
        runtimeObject.behavior.oscillate.amplitude =
            objectDef.behaviorAmplitude;
        runtimeObject.behavior.oscillate.initialPosition = objectDef.position;

        runtimeObject.behavior.spin.speed = objectDef.behaviorSpeed;
        runtimeObject.behavior.spin.axis  = objectDef.behaviorAxis;
        runtimeObject.behavior.spin.initialRotationAngle =
            object->getRotationAngle();

        runtimeObject.behavior.fly.speed           = objectDef.behaviorSpeed;
        runtimeObject.behavior.fly.direction       = objectDef.behaviorAxis;
        runtimeObject.behavior.fly.initialPosition = objectDef.position;

        runtimeObject.light.color     = objectDef.lightColor;
        runtimeObject.light.intensity = objectDef.lightIntensity;

        runtimeObjects[objectDef.id] = runtimeObject;
        runtimeObjectOrder.push_back(objectDef.id);
    }

    return true;
}

void Scene::collectActiveLightSources()
{
    // Resolve active light providers once for lit-object uniforms.
    activeLightSources.clear();
    for (const std::string& objectId : runtimeObjectOrder)
    {
        RuntimeSceneObject& runtimeObject = runtimeObjects.at(objectId);
        if (runtimeObject.core.role == SceneRole::LightSource)
        {
            activeLightSources.push_back(&runtimeObject);
        }
    }
}

Scene::PerFrameLightUniforms
Scene::collectLightUniforms(int maxLightSources) const
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
            light->core.object->getPosition();
        perFrameLightUniforms.lightColors[static_cast<std::size_t>(i)] =
            light->light.color * light->light.intensity;
    }

    return perFrameLightUniforms;
}

void Scene::ensureLightUniformNames(int maxLightSources)
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

void Scene::renderRuntimeObjects(
    const Camera& camera, const glm::mat4& projection, const glm::mat4& view,
    float sceneElapsedTime, int maxLightSources,
    const PerFrameLightUniforms& perFrameLightUniforms)
{
    ensureLightUniformNames(maxLightSources);

    std::unordered_set<unsigned int> configuredLitPrograms;
    std::unordered_set<unsigned int> configuredUnlitPrograms;
    const bool frustumCullingEnabled = renderingConfig.frustumCullingEnabled;
    const std::array<FrustumPlane, 6> frustumPlanes =
        frustumCullingEnabled ? buildFrustumPlanes(projection * view)
                              : std::array<FrustumPlane, 6>{};

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
        const RuntimeSceneObject& runtimeObject = runtimeObjects.at(objectId);
        const std::string         groupKey =
            runtimeObject.core.meshName + ":" + runtimeObject.core.materialId;

        if (groups.find(groupKey) == groups.end())
        {
            groups[groupKey] = {
                runtimeObject.core.meshName, runtimeObject.core.materialId, {}};
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
        const RuntimeSceneObject& firstObject =
            runtimeObjects.at(group.objectIds.front());
        std::shared_ptr<GpuMesh>         mesh     = firstObject.core.mesh;
        std::shared_ptr<RuntimeMaterial> material = firstObject.core.material;

        mesh->bind();
        if (firstObject.core.instanceBuffer)
        {
            firstObject.core.instanceBuffer->attachToBoundVao();
        }

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

        if (!firstObject.core.instanceBuffer)
        {
            continue;
        }

        std::vector<std::size_t> visibleInstanceIndices;
        visibleInstanceIndices.reserve(group.objectIds.size());
        for (const std::string& objectId : group.objectIds)
        {
            const RuntimeSceneObject& runtimeObject =
                runtimeObjects.at(objectId);
            if (!frustumCullingEnabled ||
                isRuntimeObjectVisible(runtimeObject, frustumPlanes))
            {
                visibleInstanceIndices.push_back(
                    runtimeObject.core.instanceIndex);
            }
        }

        if (visibleInstanceIndices.empty())
        {
            continue;
        }

        firstObject.core.instanceBuffer->prepareDraw(visibleInstanceIndices);
        const std::size_t instanceCount =
            firstObject.core.instanceBuffer->getPreparedInstanceCount();

        if (instanceCount == 0)
        {
            continue;
        }

        material->shader->setMat4("model", glm::mat4(1.0f));
        material->shader->setMat3("normalMatrix", glm::mat3(1.0f));
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

    if (perFrameLightUniforms.lightCount > 0)
    {
        // Backward-compatible single-light uniforms for legacy shaders.
        material->shader->setVec3("lightColor",
                                  perFrameLightUniforms.lightColors.front());
        material->shader->setVec3("lightPos",
                                  perFrameLightUniforms.lightPositions.front());
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

void Scene::update(float sceneElapsedTime)
{
    // Behaviors are evaluated from the spawn transform, which keeps them
    // deterministic and avoids accumulating floating-point drift.
    for (const std::string& objectId : runtimeObjectOrder)
    {
        updateRuntimeObjectBehavior(runtimeObjects.at(objectId),
                                    sceneElapsedTime);
    }
}

void Scene::updateRuntimeObjectBehavior(RuntimeSceneObject& runtimeObject,
                                        float               sceneElapsedTime)
{
    switch (runtimeObject.behavior.type)
    {
        case BehaviorType::None:
            break;
        case BehaviorType::Oscillate:
            applyBehaviorOscillate(runtimeObject, sceneElapsedTime);
            break;
        case BehaviorType::Spin:
            applyBehaviorSpin(runtimeObject, sceneElapsedTime);
            break;
        case BehaviorType::Fly:
            applyBehaviorFly(runtimeObject, sceneElapsedTime);
            break;
        default:
            break;
    }
}

void Scene::applyBehaviorOscillate(RuntimeSceneObject& runtimeObject,
                                   float               sceneElapsedTime)
{
    std::shared_ptr<Object> object = runtimeObject.core.object;
    const float             delta =
        std::sin(sceneElapsedTime * runtimeObject.behavior.oscillate.speed) *
        runtimeObject.behavior.oscillate.amplitude;
    object->setPosition(runtimeObject.behavior.oscillate.initialPosition +
                        runtimeObject.behavior.oscillate.axis * delta);

    // Sync updated transform to instance buffer
    if (runtimeObject.core.instanceBuffer)
    {
        runtimeObject.core.instanceBuffer->updateInstance(
            runtimeObject.core.instanceIndex, object->getModelMatrix());
    }
}

void Scene::applyBehaviorSpin(RuntimeSceneObject& runtimeObject,
                              float               sceneElapsedTime)
{
    std::shared_ptr<Object> object = runtimeObject.core.object;
    object->setRotation(runtimeObject.behavior.spin.initialRotationAngle +
                            runtimeObject.behavior.spin.speed *
                                sceneElapsedTime,
                        runtimeObject.behavior.spin.axis);

    // Sync updated transform to instance buffer
    if (runtimeObject.core.instanceBuffer)
    {
        runtimeObject.core.instanceBuffer->updateInstance(
            runtimeObject.core.instanceIndex, object->getModelMatrix());
    }
}

void Scene::applyBehaviorFly(RuntimeSceneObject& runtimeObject,
                             float               sceneElapsedTime)
{
    std::shared_ptr<Object> object = runtimeObject.core.object;
    const glm::vec3         direction =
        glm::normalize(runtimeObject.behavior.fly.direction);
    const glm::vec3 displacement =
        direction * (runtimeObject.behavior.fly.speed * sceneElapsedTime);
    object->setPosition(runtimeObject.behavior.fly.initialPosition +
                        displacement);

    // Sync updated transform to instance buffer
    if (runtimeObject.core.instanceBuffer)
    {
        runtimeObject.core.instanceBuffer->updateInstance(
            runtimeObject.core.instanceIndex, object->getModelMatrix());
    }
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
                         maxLightSources, perFrameLightUniforms);

    // Render scene text and runtime overlays through a dedicated presenter.
    overlayRenderer->render(textRenderer, *definition, overlayConfig,
                            infoOverlayEnabled, fps, sceneElapsedTime,
                            currentTimeSeconds);
}
