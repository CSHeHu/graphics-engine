#ifndef SCENE_H
#define SCENE_H

#include <array>
#include <glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "SceneDefinition.h"

class Camera;
class Object;
class GpuMesh;
class InstanceBuffer;
class Shader;
class AssetManager;
class TextManager;
class SceneOverlayRenderer;
class AudioManager;

/**
 * @brief Runtime scene container that renders scene content.
 */
class Scene
{
    public:
        /** @brief Construct scene runtime from parsed definition and shared
         * managers. */
        Scene(AssetManager& assetManager, SceneDefinition& definition,
              TextManager& textManager, const RenderingConfig& renderingConfig,
              AudioManager& audioManager);
        /** @brief Destroy scene runtime resources. */
        ~Scene();

        /** @brief Initialize runtime materials and objects for the active
         * definition. */
        bool init();
        /** @brief Render scene geometry and optional text overlay. */
        void render(const Camera& camera, const glm::mat4& projection,
                    const glm::mat4& view, float fps, float sceneElapsedTime,
                    const UIOverlayConfig& overlayConfig,
                    bool infoOverlayEnabled, float currentTimeSeconds);

    private:
        /** @brief Runtime material state resolved from scene configuration. */
        struct RuntimeMaterial
        {
                std::shared_ptr<Shader> shader;
                RenderMode              renderMode;
                glm::vec3               objectColor;
        };

        /** @brief Runtime render/object data shared by all scene objects. */
        struct RuntimeSceneObjectCore
        {
                std::shared_ptr<GpuMesh>         mesh;
                std::shared_ptr<Object>          object;
                std::shared_ptr<RuntimeMaterial> material;
                SceneRole                        role;
                std::string meshName;   // For grouping during render
                std::string materialId; // For grouping during render
                std::shared_ptr<InstanceBuffer>
                            instanceBuffer; // Per-mesh instance storage
                std::size_t instanceIndex;
                float       cullingRadius;
        };

        /** @brief Runtime light-emitter parameters for a scene object. */
        struct RuntimeLightState
        {
                glm::vec3 color;
                float     intensity;
        };

        /** @brief Per-behavior runtime state for oscillation. */
        struct OscillateBehaviorState
        {
                float     speed;
                glm::vec3 axis;
                float     amplitude;
                glm::vec3 initialPosition;
        };

        /** @brief Per-behavior runtime state for spinning. */
        struct SpinBehaviorState
        {
                float     speed;
                glm::vec3 axis;
                float     initialRotationAngle;
        };

        /** @brief Per-behavior runtime state for flying. */
        struct FlyBehaviorState
        {
                float     speed;
                glm::vec3 direction;
                glm::vec3 initialPosition;
        };

        /** @brief Runtime behavior state containing behavior kind and payloads.
         */
        struct RuntimeBehaviorState
        {
                BehaviorType           type;
                OscillateBehaviorState oscillate;
                SpinBehaviorState      spin;
                FlyBehaviorState       fly;
        };

        /** @brief Runtime object state used for rendering and behavior updates.
         */
        struct RuntimeSceneObject
        {
                RuntimeSceneObjectCore core;
                RuntimeBehaviorState   behavior;
                RuntimeLightState      light;
        };

        struct PerFrameLightUniforms
        {
                /** Number of active lights written into uniform arrays this
                 * frame. */
                int lightCount;
                /** World-space light base positions for active lights. */
                std::vector<glm::vec3> lightBasePositions;
                /** Light behavior types for active lights. */
                std::vector<int> lightBehaviorTypes;
                /** Light behavior speeds for active lights. */
                std::vector<float> lightBehaviorSpeeds;
                /** Light behavior axes for active lights. */
                std::vector<glm::vec3> lightBehaviorAxes;
                /** Light behavior amplitudes for active lights. */
                std::vector<float> lightBehaviorAmplitudes;
                /** Final light colors (tint * intensity) for active lights. */
                std::vector<glm::vec3> lightColors;
        };

        struct FrustumPlane
        {
                glm::vec3 normal;
                float     distance;
        };

        AssetManager&                         assets;
        TextManager&                          textRenderer;
        AudioManager&                         audio;
        SceneDefinition&                      definition;
        RenderingConfig                       renderingConfig;
        std::unique_ptr<SceneOverlayRenderer> overlayRenderer;

        /** Runtime material map keyed by material id. */
        std::unordered_map<std::string, std::shared_ptr<RuntimeMaterial>>
            runtimeMaterials;
        /** Instance buffer per unique mesh name for GPU-resident transforms. */
        std::unordered_map<std::string, std::shared_ptr<InstanceBuffer>>
            instanceBuffers;
        /** Runtime object map keyed by object id. */
        std::unordered_map<std::string, std::shared_ptr<RuntimeSceneObject>>
            runtimeObjects;
        /** Deterministic object iteration order matching scene definition
         * order. */
        std::vector<std::string>                         runtimeObjectOrder;
        std::vector<std::shared_ptr<RuntimeSceneObject>> activeLightSources;

        /** Build shader-backed material instances for the current scene. */
        bool initializeRuntimeMaterials();
        /** Build runtime objects and apply initial transforms from scene data.
         */
        bool initializeRuntimeObjects();
        /** Refresh cached pointers to objects that act as lights. */
        void collectActiveLightSources();

        /** Collect per-frame light uniforms from active light sources. */
        PerFrameLightUniforms collectLightUniforms(int maxLightSources) const;
        /** Draw all scene objects with per-shader uniform setup. */
        void renderRuntimeObjects(
            const Camera& camera, const glm::mat4& projection,
            const glm::mat4& view, float sceneElapsedTime,
            const PerFrameLightUniforms& perFrameLightUniforms);
        /** Configure shared per-frame uniforms for lit material shaders. */
        void configureLitShaderUniforms(
            const std::shared_ptr<RuntimeMaterial>& material,
            const Camera& camera, const glm::mat4& projection,
            const glm::mat4& view, float sceneElapsedTime,
            const PerFrameLightUniforms& perFrameLightUniforms) const;
        /** Configure shared per-frame uniforms for light-emitter shaders. */
        void configureEmitterShaderUniforms(
            const std::shared_ptr<RuntimeMaterial>& material,
            const glm::mat4& projection, const glm::mat4& view,
            float sceneElapsedTime) const;

        std::array<FrustumPlane, 6>
             buildFrustumPlanes(const glm::mat4& projection,
                                const glm::mat4& view) const;
        bool isRuntimeObjectVisible(
            const RuntimeSceneObject&          runtimeObject,
            const std::array<FrustumPlane, 6>& frustumPlanes) const;
};

#endif // SCENE_H
