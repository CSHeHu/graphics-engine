#ifndef SCENE_H
#define SCENE_H

#include <array>
#include <cstddef>
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
 * @brief Runtime scene container that updates behaviors and renders scene
 * content.
 */
class Scene
{
    public:
        /** @brief Construct scene runtime from parsed definition and shared
         * managers. */
        Scene(AssetManager&                    assetManager,
              std::shared_ptr<SceneDefinition> definition,
              TextManager& textManager, const RenderingConfig& renderingConfig,
              AudioManager& audioManager);
        /** @brief Destroy scene runtime resources. */
        ~Scene();

        /** @brief Initialize runtime materials and objects for the active
         * definition. */
        bool init();
        /** @brief Update behavior-driven transforms for the current scene time.
         */
        void update(float sceneElapsedTime);
        /** @brief Render scene geometry and optional text overlay. */
        void render(const Camera& camera, const glm::mat4& projection,
                    const glm::mat4& view, float fps, float sceneElapsedTime,
                    const UIOverlayConfig& overlayConfig,
                    bool infoOverlayEnabled, float currentTimeSeconds);

    private:
        enum class MatrixColumn : std::size_t
        {
            X = 0,
            Y = 1,
            Z = 2,
            W = 3,
        };

        struct FrustumPlane
        {
                glm::vec3                    normal;
                float                        distance;
                static constexpr std::size_t kFrustumPlaneCount = 6;
        };

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
                std::string meshName;      // For grouping during render
                std::string materialId;    // For grouping during render
                std::size_t instanceIndex; // Index in InstanceBuffer
                std::shared_ptr<InstanceBuffer>
                    instanceBuffer; // Per-mesh instance storage
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
                /** World-space light positions for active lights. */
                std::vector<glm::vec3> lightPositions;
                /** Final light colors (tint * intensity) for active lights. */
                std::vector<glm::vec3> lightColors;
        };

        AssetManager&                         assets;
        TextManager&                          textRenderer;
        AudioManager&                         audio;
        std::shared_ptr<SceneDefinition>      definition;
        RenderingConfig                       renderingConfig;
        std::unique_ptr<SceneOverlayRenderer> overlayRenderer;

        /** Runtime material map keyed by material id. */
        std::unordered_map<std::string, std::shared_ptr<RuntimeMaterial>>
            runtimeMaterials;
        /** Instance buffer per unique mesh name for GPU-resident transforms. */
        std::unordered_map<std::string, std::shared_ptr<InstanceBuffer>>
            instanceBuffers;
        /** Runtime object map keyed by object id. */
        std::unordered_map<std::string, RuntimeSceneObject> runtimeObjects;
        /** Deterministic object iteration order matching scene definition
         * order. */
        std::vector<std::string>         runtimeObjectOrder;
        std::vector<RuntimeSceneObject*> activeLightSources;

        /** Build shader-backed material instances for the current scene. */
        bool initializeRuntimeMaterials();
        /** Build runtime objects and apply initial transforms from scene data.
         */
        bool initializeRuntimeObjects();
        /** Refresh cached pointers to objects that act as lights. */
        void collectActiveLightSources();

        /** Collect per-frame light uniforms from active light sources. */
        PerFrameLightUniforms collectLightUniforms(int maxLightSources) const;
        /** Apply configured behavior for one runtime object. */
        void updateRuntimeObjectBehavior(RuntimeSceneObject& runtimeObject,
                                         float               sceneElapsedTime);
        /** Apply oscillation behavior around initial position. */
        void applyBehaviorOscillate(RuntimeSceneObject& runtimeObject,
                                    float               sceneElapsedTime);
        /** Apply spin behavior around configured axis. */
        void applyBehaviorSpin(RuntimeSceneObject& runtimeObject,
                               float               sceneElapsedTime);
        /** Apply linear fly behavior along configured direction. */
        void applyBehaviorFly(RuntimeSceneObject& runtimeObject,
                              float               sceneElapsedTime);
        /** Draw all scene objects with per-shader uniform setup. */
        void renderRuntimeObjects(
            const Camera& camera, const glm::mat4& projection,
            const glm::mat4& view, float sceneElapsedTime, int maxLightSources,
            const PerFrameLightUniforms& perFrameLightUniforms);
        /** Build view frustum planes for optional CPU culling. */
        std::array<FrustumPlane, FrustumPlane::kFrustumPlaneCount>
        buildFrustumPlanes(const glm::mat4& viewProjection) const;
        /** Test whether one runtime object intersects the current frustum. */
        bool isRuntimeObjectVisible(
            const RuntimeSceneObject& runtimeObject,
            const std::array<FrustumPlane, FrustumPlane::kFrustumPlaneCount>&
                frustumPlanes) const;
        /** Compute a conservative bounding sphere radius from object scale. */
        float computeBoundingRadius(const Object& object) const;
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
};

#endif // SCENE_H
