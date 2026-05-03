#ifndef SCENEDEFINITION_H
#define SCENEDEFINITION_H

#include <cstddef>
#include <glm.hpp>
#include <string>
#include <vector>

#include "Object.h"

/**
 * @brief Roles that a scene object can play in the scene.
 */
enum class SceneRole
{
    None,        /**< No special role. */
    LightSource, /**< Acts as a light source. */
    LightTarget, /**< Target for light. */
    Ground,      /**< Ground object. */
};

/**
 * @brief Rendering modes for scene objects.
 */
enum class RenderMode
{
    LightSource, /**< Render as a light source. */
    Lit,         /**< Render as a lit object. */
};

/**
 * @brief Types of behavior that can be applied to scene objects.
 */
enum class BehaviorType
{
    None,      /**< No behavior. */
    Oscillate, /**< Oscillating movement. */
    Spin,      /**< Spinning behavior. */
    Fly,       /**< Flying behavior. */
};

/**
 * @brief Camera control modes.
 */
enum class CameraMode
{
    Manual,   /**< Manual camera control. */
    Scripted, /**< Scripted camera route. */
};

/**
 * @brief Window display modes.
 */
enum class WindowMode
{
    Windowed,   /**< Windowed mode. */
    Fullscreen, /**< Fullscreen mode. */
};

/**
 * @brief Represents a single camera keyframe for scripted camera movement.
 */
struct CameraKeyframe
{
        float     timeSeconds; /**< Time of the keyframe in seconds. */
        glm::vec3 position;    /**< Camera position at this keyframe. */
        glm::vec3 lookAt;      /**< Target point to look at. */
};

/**
 * @brief Defines a camera route for scripted camera movement.
 */
struct CameraRouteDefinition
{
        CameraMode mode =
            CameraMode::Manual; /**< Camera mode (manual/scripted). */
        bool loop = true;       /**< Whether the route should loop. */
        /** Time-ordered camera keyframes for scripted mode. */
        std::vector<CameraKeyframe> keyframes;
};

/**
 * @brief Material definition for scene objects.
 */
struct MaterialDefinition
{
        std::string id;                 /**< Material identifier. */
        std::string vertexShaderPath;   /**< Path to vertex shader. */
        std::string fragmentShaderPath; /**< Path to fragment shader. */
        std::string geometryShaderPath; /**< Path to geometry shader. */
        RenderMode  renderMode;         /**< Rendering mode. */
        glm::vec3   objectColor;        /**< Base color. */
};

/**
 * @brief Definition of an object in the scene.
 */
struct SceneObjectDefinition
{
        std::string          id;       /**< Object identifier. */
        SceneRole            role;     /**< Role in the scene. */
        std::string          meshName; /**< Mesh file name. */
        Object::VertexLayout layout;   /**< Vertex layout type. */
        glm::vec3            position; /**< Initial position. */
        glm::vec3 rotation; /**< Euler angles in radians (pitch, yaw, roll). */
        glm::vec3 scale;    /**< Scale vector. */
        std::string materialId; /**< Material identifier. */

        BehaviorType behavior;
        float        behaviorSpeed;
        glm::vec3    behaviorAxis;
        float        behaviorAmplitude;

        glm::vec3 lightColor;
        float     lightIntensity;
};

struct TextDefinition
{
        std::string text;
        float       x;
        float       y;
        float       scale;
        glm::vec3   color;
};

struct SceneAudioDefinition
{
        std::string musicPath;
        int         loops                 = -1;
        bool        continueOnSceneChange = false;
};

struct UIOverlayConfig
{
        bool        enabled;
        std::string fontPath;
        std::string vertexShaderPath;
        std::string fragmentShaderPath;
        float       x;
        float       y;
        float       scale;
        float       lineSpacing;
        /** Overlay stat identifiers to render in configured order. */
        std::vector<std::string> stats;
};

struct WindowConfig
{
        WindowMode mode;
        int        width;
        int        height;
        bool       cursorCaptured;
        bool       vsyncEnabled;
};

struct OpenGLConfig
{
        int contextVersionMajor;
        int contextVersionMinor;
};

struct CameraDefaultsConfig
{
        float     yaw;
        float     pitch;
        float     speed;
        float     sensitivity;
        float     zoom;
        glm::vec3 position;
};

struct AssetPathsConfig
{
        std::string scenesPath;
        std::string meshesPath;
        std::string shadersPath;
};

struct InputConfig
{
        bool cameraControlsEnabled;
        int  keyEscape;
        int  keyMoveForward;
        int  keyMoveBackward;
        int  keyMoveLeft;
        int  keyMoveRight;
        int  keyMoveUp;
        int  keyMoveDown;
        int  keyToggleCameraMode;
        int  keyToggleInfoOverlay;
        int  keyTogglePause;
        int  keyStepTimeBackward;
        int  keyStepTimeForward;
};

struct RenderingConfig
{
        std::size_t positionNormalStride;
        int         maxLightSources;
        float       nearPlane;
        float       farPlane;
        bool        depthTestEnabled;
        bool        blendEnabled;
};

struct RuntimeConfig
{
        std::string          windowTitle;
        OpenGLConfig         opengl;
        CameraDefaultsConfig camera;
        AssetPathsConfig     assets;
        InputConfig          input;
        RenderingConfig      rendering;
};

struct SceneDefinition
{
        std::string           name;
        CameraRouteDefinition camera;
        /** Optional scene music played while this scene is active. */
        SceneAudioDefinition audio;
        /** Material definitions referenced by scene objects. */
        std::vector<MaterialDefinition> materials;
        /** Scene object definitions instantiated for runtime rendering. */
        std::vector<SceneObjectDefinition> objects;
        /** Optional static text overlays rendered with scene. */
        std::vector<TextDefinition> texts;
};

#endif // SCENEDEFINITION_H
