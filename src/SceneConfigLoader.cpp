#include "SceneConfigLoader.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

template <typename T>
T SceneConfigLoader::parseEnumValue(
    const std::string& value, const std::unordered_map<std::string, T>& mapping,
    const char* typeName) const
{
    const auto it = mapping.find(value);
    if (it == mapping.end())
    {
        throw std::runtime_error(std::string("Unsupported ") + typeName +
                                 " value: " + value);
    }

    return it->second;
}

std::ifstream SceneConfigLoader::openAssetFile(const std::string& path)
{
    std::ifstream file(path);
    if (file.is_open())
    {
        return file;
    }

    file.clear();
    file.open("../" + path);
    return file;
}

SceneId SceneConfigLoader::parseSceneId(const std::string& value)
{
    return parseEnumValue(value, sceneIdMap, "SceneId");
}

RenderMode SceneConfigLoader::parseRenderMode(const std::string& value)
{
    return parseEnumValue(value, renderModeMap, "RenderMode");
}

SceneRole SceneConfigLoader::parseSceneRole(const std::string& value)
{
    return parseEnumValue(value, sceneRoleMap, "SceneRole");
}

BehaviorType SceneConfigLoader::parseBehaviorType(const std::string& value)
{
    return parseEnumValue(value, behaviorTypeMap, "BehaviorType");
}

CameraMode SceneConfigLoader::parseCameraMode(const std::string& value)
{
    return parseEnumValue(value, cameraModeMap, "CameraMode");
}

WindowMode SceneConfigLoader::parseWindowMode(const std::string& value)
{
    return parseEnumValue(value, windowModeMap, "WindowMode");
}

VertexLayout SceneConfigLoader::parseVertexLayout(const std::string& value)
{
    return parseEnumValue(value, vertexLayoutMap, "vertex layout");
}

glm::vec3 SceneConfigLoader::parseVec3(float x, float y, float z)
{
    return glm::vec3(x, y, z);
}

SceneDefinition
SceneConfigLoader::parseSceneDefinition(const std::string& sceneFilePath)
{
    std::ifstream file = openAssetFile(sceneFilePath);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open scene file: " + sceneFilePath);
    }

    nlohmann::json root;
    file >> root;

    SceneDefinition definition;
    definition.name = root.at("name").get<std::string>();

    if (root.contains("camera"))
    {
        const nlohmann::json& cameraJson = root.at("camera");
        definition.camera.mode =
            parseCameraMode(cameraJson.value("mode", "Manual"));
        definition.camera.loop = cameraJson.value("loop", true);

        if (cameraJson.contains("keyframes"))
        {
            for (const nlohmann::json& keyframeJson :
                 cameraJson.at("keyframes"))
            {
                CameraKeyframe keyframe;
                keyframe.timeSeconds = keyframeJson.at("t").get<float>();

                const nlohmann::json& position = keyframeJson.at("position");
                if (!position.is_array() || position.size() != 3)
                {
                    throw std::runtime_error("Expected vec3 array for field: "
                                             "camera.keyframes.position");
                }
                keyframe.position = parseVec3(position[0].get<float>(),
                                              position[1].get<float>(),
                                              position[2].get<float>());

                const nlohmann::json& lookAt = keyframeJson.at("lookAt");
                if (!lookAt.is_array() || lookAt.size() != 3)
                {
                    throw std::runtime_error("Expected vec3 array for field: "
                                             "camera.keyframes.lookAt");
                }
                keyframe.lookAt =
                    parseVec3(lookAt[0].get<float>(), lookAt[1].get<float>(),
                              lookAt[2].get<float>());

                definition.camera.keyframes.push_back(keyframe);
            }

            std::sort(definition.camera.keyframes.begin(),
                      definition.camera.keyframes.end(),
                      [](const CameraKeyframe& a, const CameraKeyframe& b)
                      { return a.timeSeconds < b.timeSeconds; });
        }
    }

    if (root.contains("audio"))
    {
        const nlohmann::json& audioJson = root.at("audio");
        definition.audio.musicPath      = audioJson.value("musicPath", "");
        definition.audio.loops          = audioJson.value("loops", -1);
        definition.audio.continueOnSceneChange =
            audioJson.value("continueOnSceneChange", false);
    }

    for (const nlohmann::json& materialJson : root.at("materials"))
    {
        MaterialDefinition material;
        material.id = materialJson.at("id").get<std::string>();
        material.vertexShaderPath =
            materialJson.at("vertexShaderPath").get<std::string>();
        material.fragmentShaderPath =
            materialJson.at("fragmentShaderPath").get<std::string>();
        material.geometryShaderPath =
            materialJson.value("geometryShaderPath", "");
        material.renderMode =
            parseRenderMode(materialJson.at("renderMode").get<std::string>());
        const nlohmann::json& objectColor = materialJson.at("objectColor");
        if (!objectColor.is_array() || objectColor.size() != 3)
        {
            throw std::runtime_error(
                "Expected vec3 array for field: materials.objectColor");
        }
        material.objectColor =
            parseVec3(objectColor[0].get<float>(), objectColor[1].get<float>(),
                      objectColor[2].get<float>());
        definition.materials.push_back(material);
    }

    for (const nlohmann::json& objectJson : root.at("objects"))
    {
        SceneObjectDefinition object;
        object.id   = objectJson.at("id").get<std::string>();
        object.role = parseSceneRole(objectJson.at("role").get<std::string>());
        object.meshName = objectJson.at("meshName").get<std::string>();
        object.layout =
            parseVertexLayout(objectJson.at("layout").get<std::string>());
        const nlohmann::json& position = objectJson.at("position");
        if (!position.is_array() || position.size() != 3)
        {
            throw std::runtime_error(
                "Expected vec3 array for field: objects.position");
        }
        object.position =
            parseVec3(position[0].get<float>(), position[1].get<float>(),
                      position[2].get<float>());

        const nlohmann::json rotation = objectJson.value(
            "rotation", nlohmann::json::array({0.0f, 0.0f, 0.0f}));
        if (!rotation.is_array() || rotation.size() != 3)
        {
            throw std::runtime_error(
                "Expected vec3 array for field: objects.rotation");
        }
        object.rotation =
            parseVec3(rotation[0].get<float>(), rotation[1].get<float>(),
                      rotation[2].get<float>());

        const nlohmann::json scale = objectJson.value(
            "scale", nlohmann::json::array({1.0f, 1.0f, 1.0f}));
        if (!scale.is_array() || scale.size() != 3)
        {
            throw std::runtime_error(
                "Expected vec3 array for field: objects.scale");
        }
        object.scale = parseVec3(scale[0].get<float>(), scale[1].get<float>(),
                                 scale[2].get<float>());

        object.materialId = objectJson.at("materialId").get<std::string>();
        object.behavior =
            parseBehaviorType(objectJson.at("behavior").get<std::string>());
        object.behaviorSpeed = objectJson.value("behaviorSpeed", 0.0f);
        const nlohmann::json& behaviorAxis = objectJson.at("behaviorAxis");
        if (!behaviorAxis.is_array() || behaviorAxis.size() != 3)
        {
            throw std::runtime_error(
                "Expected vec3 array for field: objects.behaviorAxis");
        }
        object.behaviorAxis      = parseVec3(behaviorAxis[0].get<float>(),
                                             behaviorAxis[1].get<float>(),
                                             behaviorAxis[2].get<float>());
        object.behaviorAmplitude = objectJson.value("behaviorAmplitude", 0.0f);

        const nlohmann::json lightColor = objectJson.value(
            "lightColor", nlohmann::json::array({1.0f, 1.0f, 1.0f}));
        if (!lightColor.is_array() || lightColor.size() != 3)
        {
            throw std::runtime_error(
                "Expected vec3 array for field: objects.lightColor");
        }
        object.lightColor =
            parseVec3(lightColor[0].get<float>(), lightColor[1].get<float>(),
                      lightColor[2].get<float>());
        object.lightIntensity = objectJson.value("lightIntensity", 1.0f);

        // Support explicit instance lists or simple circular instance
        // spawning. If the scene author provides an "instances" array of
        // vec3 positions, expand the template into multiple objects at
        // those positions. Otherwise, if an integer "instanceCount" is
        // provided, spawn that many copies evenly around a circle with
        // optional "instanceRadius" (defaults to 5.0).
        if (objectJson.contains("instances"))
        {
            const nlohmann::json& instancesJson = objectJson.at("instances");
            if (!instancesJson.is_array())
            {
                throw std::runtime_error(
                    "Expected array for field: objects.instances");
            }
            std::size_t idx = 0;
            for (const auto& posJson : instancesJson)
            {
                if (!posJson.is_array() || posJson.size() != 3)
                {
                    throw std::runtime_error(
                        "Expected vec3 array for objects.instances entry");
                }
                SceneObjectDefinition inst = object;
                inst.id = object.id + "_" + std::to_string(idx++);
                inst.position =
                    parseVec3(posJson[0].get<float>(), posJson[1].get<float>(),
                              posJson[2].get<float>());
                definition.objects.push_back(inst);
            }
        }
        else if (objectJson.contains("instanceCount"))
        {
            const int instanceCount = objectJson.at("instanceCount").get<int>();
            const float radius      = objectJson.value("instanceRadius", 5.0f);
            if (instanceCount <= 0)
            {
                // nothing to spawn
            }
            else
            {
                for (int i = 0; i < instanceCount; ++i)
                {
                    const float angle = 2.0f * static_cast<float>(M_PI) *
                                        static_cast<float>(i) /
                                        static_cast<float>(instanceCount);
                    SceneObjectDefinition inst = object;
                    inst.id       = object.id + "_" + std::to_string(i);
                    inst.position = object.position +
                                    glm::vec3(std::cos(angle) * radius, 0.0f,
                                              std::sin(angle) * radius);
                    definition.objects.push_back(inst);
                }
            }
        }
        else
        {
            definition.objects.push_back(object);
        }
    }

    // Parse text overlays
    if (root.contains("texts"))
    {
        for (const nlohmann::json& textJson : root.at("texts"))
        {
            TextDefinition text;
            text.text  = textJson.at("text").get<std::string>();
            text.x     = textJson.at("x").get<float>();
            text.y     = textJson.at("y").get<float>();
            text.scale = textJson.at("scale").get<float>();
            const nlohmann::json& color = textJson.at("color");
            if (!color.is_array() || color.size() != 3)
            {
                throw std::runtime_error(
                    "Expected vec3 array for field: texts.color");
            }
            text.color = parseVec3(color[0].get<float>(), color[1].get<float>(),
                                   color[2].get<float>());
            definition.texts.push_back(text);
        }
    }

    return definition;
}

UIOverlayConfig
SceneConfigLoader::parseUIOverlayConfig(const nlohmann::json& json)
{
    UIOverlayConfig config;
    config.enabled          = json.at("enabled").get<bool>();
    config.fontPath         = json.at("fontPath").get<std::string>();
    config.vertexShaderPath = json.at("vertexShaderPath").get<std::string>();
    config.fragmentShaderPath =
        json.at("fragmentShaderPath").get<std::string>();
    config.x           = json.at("x").get<float>();
    config.y           = json.at("y").get<float>();
    config.scale       = json.at("scale").get<float>();
    config.lineSpacing = json.at("lineSpacing").get<float>();

    if (json.contains("stats"))
    {
        for (const auto& stat : json.at("stats"))
        {
            config.stats.push_back(stat.get<std::string>());
        }
    }

    return config;
}

WindowConfig SceneConfigLoader::parseWindowConfig(const nlohmann::json& json)
{
    WindowConfig config;
    config.mode           = parseWindowMode(json.at("mode").get<std::string>());
    config.width          = json.at("width").get<int>();
    config.height         = json.at("height").get<int>();
    config.cursorCaptured = json.at("cursorCaptured").get<bool>();
    config.vsyncEnabled   = json.at("vsyncEnabled").get<bool>();
    return config;
}

RuntimeConfig SceneConfigLoader::parseRuntimeConfig(const nlohmann::json& json)
{
    RuntimeConfig config;

    // Global runtime settings centralize values that were previously split
    // across constants.
    config.windowTitle = json.at("windowTitle").get<std::string>();

    const nlohmann::json& openglJson = json.at("opengl");
    config.opengl.contextVersionMajor =
        openglJson.at("contextVersionMajor").get<int>();
    config.opengl.contextVersionMinor =
        openglJson.at("contextVersionMinor").get<int>();

    const nlohmann::json& cameraJson = json.at("cameraDefaults");
    config.camera.yaw                = cameraJson.at("yaw").get<float>();
    config.camera.pitch              = cameraJson.at("pitch").get<float>();
    config.camera.speed              = cameraJson.at("speed").get<float>();
    config.camera.sensitivity = cameraJson.at("sensitivity").get<float>();
    config.camera.zoom        = cameraJson.at("zoom").get<float>();
    const nlohmann::json& cameraPosition = cameraJson.at("position");
    if (!cameraPosition.is_array() || cameraPosition.size() != 3)
    {
        throw std::runtime_error(
            "Expected vec3 array for field: config.cameraDefaults.position");
    }
    config.camera.position = parseVec3(cameraPosition[0].get<float>(),
                                       cameraPosition[1].get<float>(),
                                       cameraPosition[2].get<float>());

    const nlohmann::json& assetsJson = json.at("assets");
    config.assets.scenesPath  = assetsJson.at("scenesPath").get<std::string>();
    config.assets.meshesPath  = assetsJson.at("meshesPath").get<std::string>();
    config.assets.shadersPath = assetsJson.at("shadersPath").get<std::string>();

    const nlohmann::json& inputJson = json.at("input");
    config.input.cameraControlsEnabled =
        inputJson.at("cameraControlsEnabled").get<bool>();
    config.input.keyEscape       = inputJson.at("escape").get<int>();
    config.input.keyMoveForward  = inputJson.at("moveForward").get<int>();
    config.input.keyMoveBackward = inputJson.at("moveBackward").get<int>();
    config.input.keyMoveLeft     = inputJson.at("moveLeft").get<int>();
    config.input.keyMoveRight    = inputJson.at("moveRight").get<int>();
    config.input.keyMoveUp       = inputJson.at("moveUp").get<int>();
    config.input.keyMoveDown     = inputJson.at("moveDown").get<int>();
    config.input.keyToggleCameraMode =
        inputJson.at("toggleCameraMode").get<int>();
    config.input.keyToggleInfoOverlay =
        inputJson.at("toggleInfoOverlay").get<int>();
    config.input.keyTogglePause = inputJson.at("togglePause").get<int>();
    config.input.keyStepTimeBackward =
        inputJson.at("stepTimeBackward").get<int>();
    config.input.keyStepTimeForward =
        inputJson.at("stepTimeForward").get<int>();
    config.input.keyResetTime = inputJson.at("resetTime").get<int>();

    const nlohmann::json& renderingJson = json.at("rendering");
    config.rendering.positionNormalStride =
        renderingJson.at("positionNormalStride").get<std::size_t>();
    config.rendering.maxLightSources =
        renderingJson.at("maxLightSources").get<int>();
    config.rendering.nearPlane = renderingJson.at("nearPlane").get<float>();
    config.rendering.farPlane  = renderingJson.at("farPlane").get<float>();
    config.rendering.depthTestEnabled =
        renderingJson.at("depthTestEnabled").get<bool>();
    config.rendering.blendEnabled =
        renderingJson.at("blendEnabled").get<bool>();
    config.rendering.frustumCullingEnabled =
        renderingJson.value("frustumCullingEnabled", false);

    return config;
}

void SceneConfigLoader::ensureLoaded()
{
    if (loaded)
    {
        return;
    }

    std::ifstream file = openAssetFile("assets/scenes/scene_config.json");
    if (!file.is_open())
    {
        throw std::runtime_error(
            "Failed to open assets/scenes/scene_config.json");
    }

    nlohmann::json root;
    file >> root;

    sceneCycle.clear();
    sceneDefinitions.clear();
    runtimeConfig = parseRuntimeConfig(root.at("config"));

    // Parse UI overlay config
    if (root.contains("ui") && root.at("ui").contains("infoOverlay"))
    {
        uiOverlayConfig = parseUIOverlayConfig(root.at("ui").at("infoOverlay"));
    }

    if (root.contains("ui") && root.at("ui").contains("window"))
    {
        windowConfig = parseWindowConfig(root.at("ui").at("window"));
    }
    else
    {
        throw std::runtime_error("Missing required ui.window configuration");
    }

    for (const nlohmann::json& entryJson : root.at("registry"))
    {
        const SceneId id = parseSceneId(entryJson.at("id").get<std::string>());
        const std::string filePath = entryJson.at("file").get<std::string>();

        sceneDefinitions[static_cast<int>(id)] =
            std::make_shared<SceneDefinition>(parseSceneDefinition(filePath));
    }

    for (const nlohmann::json& cycleJson : root.at("cycle"))
    {
        SceneCycleEntry entry;
        entry.id = parseSceneId(cycleJson.at("id").get<std::string>());
        entry.durationSeconds = cycleJson.at("durationSeconds").get<float>();
        sceneCycle.push_back(entry);
    }

    loaded = true;
}

const std::vector<SceneCycleEntry>& SceneConfigLoader::getDefaultSceneCycle()
{
    ensureLoaded();
    return sceneCycle;
}

bool SceneConfigLoader::tryCreateSceneDefinition(
    SceneId id, std::shared_ptr<SceneDefinition>& outDefinition)
{
    ensureLoaded();

    const auto it = sceneDefinitions.find(static_cast<int>(id));
    if (it == sceneDefinitions.end())
    {
        return false;
    }

    outDefinition = it->second;
    return true;
}

const UIOverlayConfig& SceneConfigLoader::getUIOverlayConfig()
{
    ensureLoaded();
    return uiOverlayConfig;
}

const WindowConfig& SceneConfigLoader::getWindowConfig()
{
    ensureLoaded();
    return windowConfig;
}

const RuntimeConfig& SceneConfigLoader::getRuntimeConfig()
{
    ensureLoaded();
    return runtimeConfig;
}
