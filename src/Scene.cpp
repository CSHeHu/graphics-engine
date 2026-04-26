#include "Scene.h"

#include <algorithm>
#include <cmath>
#include <glm.hpp>
#include <iostream>
#include <unordered_set>
#include <utility>

#include "AssetManager.h"
#include "Camera.h"
#include "Object.h"
#include "SceneDefinition.h"
#include "SceneDefinitions.h"
#include "SceneOverlayRenderer.h"
#include "Shader.h"
#include "TextManager.h"

Scene::Scene(AssetManager&                    assetManager,
             std::shared_ptr<SceneDefinition> definitionValue,
             TextManager&                     textManager)
    : assets(assetManager), textRenderer(textManager),
      definition(std::move(definitionValue)), lightUniformNameTable{0, {}, {}},
      overlayRenderer(std::make_unique<SceneOverlayRenderer>())
{
}

Scene::~Scene()
{
    runtimeMaterials.clear();
    runtimeObjects.clear();
    runtimeObjectOrder.clear();
    activeLightSources.clear();
}

bool Scene::init()
{
    runtimeMaterials.clear();
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
            material->shader, vertices, indices, objectDef.position,
            objectDef.scale, objectDef.layout);

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
        runtimeObject.core.object     = object;
        runtimeObject.core.material   = material;
        runtimeObject.core.indexCount = indexCount;
        runtimeObject.core.role       = objectDef.role;

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

void Scene::refreshActiveLightSources()
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
            light->core.object->getPosition();
        perFrameLightUniforms.lightColors[static_cast<std::size_t>(i)] =
            light->light.color * light->light.intensity;
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

void Scene::renderRuntimeObjects(
    const Camera& camera, const glm::mat4& projection, const glm::mat4& view,
    float sceneElapsedTime, int maxLightSources,
    const PerFrameLightUniforms& perFrameLightUniforms)
{
    ensureLightUniformNameTable(maxLightSources);

    std::unordered_set<unsigned int> configuredLitPrograms;
    std::unordered_set<unsigned int> configuredUnlitPrograms;

    // Render all objects with shared camera matrices and mode-specific
    // uniforms.
    for (const std::string& objectId : runtimeObjectOrder)
    {
        const RuntimeSceneObject& runtimeObject = runtimeObjects.at(objectId);

        std::shared_ptr<RuntimeMaterial> material = runtimeObject.core.material;
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
                                      runtimeObject.light.color *
                                          runtimeObject.light.intensity);
        }

        drawRuntimeObject(runtimeObject);
    }
}

void Scene::configureLitShaderPerFrame(
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
            applyBehaviorNone(runtimeObject, sceneElapsedTime);
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
            applyBehaviorNone(runtimeObject, sceneElapsedTime);
            break;
    }
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
    std::shared_ptr<Object> object = runtimeObject.core.object;
    const float             delta =
        std::sin(sceneElapsedTime * runtimeObject.behavior.oscillate.speed) *
        runtimeObject.behavior.oscillate.amplitude;
    object->setPosition(runtimeObject.behavior.oscillate.initialPosition +
                        runtimeObject.behavior.oscillate.axis * delta);
}

void Scene::applyBehaviorSpin(RuntimeSceneObject& runtimeObject,
                              float               sceneElapsedTime)
{
    std::shared_ptr<Object> object = runtimeObject.core.object;
    object->setRotation(runtimeObject.behavior.spin.initialRotationAngle +
                            runtimeObject.behavior.spin.speed *
                                sceneElapsedTime,
                        runtimeObject.behavior.spin.axis);
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
}

void Scene::drawRuntimeObject(const RuntimeSceneObject& runtimeObject) const
{
    std::shared_ptr<Object>          object   = runtimeObject.core.object;
    std::shared_ptr<RuntimeMaterial> material = runtimeObject.core.material;
    glBindVertexArray(object->getVAO());
    glm::mat4 model = object->getModelMatrix();
    material->shader->setMat4("model", model);
    const glm::mat3 normalMatrix =
        glm::mat3(glm::transpose(glm::inverse(model)));
    material->shader->setMat3("normalMatrix", normalMatrix);
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(runtimeObject.core.indexCount),
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

    // Render scene text and runtime overlays through a dedicated presenter.
    overlayRenderer->render(textRenderer, *definition, overlayConfig,
                            infoOverlayEnabled, fps, sceneElapsedTime,
                            currentTimeSeconds);
}
