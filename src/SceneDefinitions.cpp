#include "SceneDefinitions.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

namespace
{
    std::vector<SceneRegistryEntry> gSceneRegistry;
    std::vector<SceneCycleEntry> gSceneCycle;
    std::unordered_map<int, SceneDefinition> gSceneDefinitions;
    bool gLoaded = false;

    std::ifstream openAssetFile(const std::string &path)
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

    SceneId parseSceneId(const std::string &value)
    {
        if (value == "Basic")
        {
            return SceneId::Basic;
        }
        if (value == "Alternate")
        {
            return SceneId::Alternate;
        }

        throw std::runtime_error("Unsupported SceneId value: " + value);
    }

    RenderMode parseRenderMode(const std::string &value)
    {
        if (value == "LightSource")
        {
            return RenderMode::LightSource;
        }
        if (value == "Lit")
        {
            return RenderMode::Lit;
        }

        throw std::runtime_error("Unsupported RenderMode value: " + value);
    }

    SceneRole parseSceneRole(const std::string &value)
    {
        if (value == "None")
        {
            return SceneRole::None;
        }
        if (value == "LightSource")
        {
            return SceneRole::LightSource;
        }
        if (value == "LightTarget")
        {
            return SceneRole::LightTarget;
        }
        if (value == "Ground")
        {
            return SceneRole::Ground;
        }

        throw std::runtime_error("Unsupported SceneRole value: " + value);
    }

    BehaviorType parseBehaviorType(const std::string &value)
    {
        if (value == "None")
        {
            return BehaviorType::None;
        }
        if (value == "Oscillate")
        {
            return BehaviorType::Oscillate;
        }
        if (value == "Spin")
        {
            return BehaviorType::Spin;
        }

        throw std::runtime_error("Unsupported BehaviorType value: " + value);
    }

    Object::VertexLayout parseVertexLayout(const std::string &value)
    {
        if (value == "PositionUV")
        {
            return Object::VertexLayout::PositionUV;
        }
        if (value == "PositionNormal")
        {
            return Object::VertexLayout::PositionNormal;
        }

        throw std::runtime_error("Unsupported vertex layout value: " + value);
    }

    glm::vec3 parseVec3(const nlohmann::json &value, const std::string &fieldName)
    {
        if (!value.is_array() || value.size() != 3)
        {
            throw std::runtime_error("Expected vec3 array for field: " + fieldName);
        }

        return glm::vec3(value[0].get<float>(), value[1].get<float>(), value[2].get<float>());
    }

    SceneDefinition parseSceneDefinition(const std::string &sceneFilePath)
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

        for (const nlohmann::json &materialJson : root.at("materials"))
        {
            MaterialDefinition material;
            material.id = materialJson.at("id").get<std::string>();
            material.vertexShaderPath = materialJson.at("vertexShaderPath").get<std::string>();
            material.fragmentShaderPath = materialJson.at("fragmentShaderPath").get<std::string>();
            material.geometryShaderPath = materialJson.value("geometryShaderPath", "");
            material.renderMode = parseRenderMode(materialJson.at("renderMode").get<std::string>());
            material.objectColor = parseVec3(materialJson.at("objectColor"), "materials.objectColor");
            definition.materials.push_back(material);
        }

        for (const nlohmann::json &objectJson : root.at("objects"))
        {
            SceneObjectDefinition object;
            object.id = objectJson.at("id").get<std::string>();
            object.role = parseSceneRole(objectJson.at("role").get<std::string>());
            object.meshName = objectJson.at("meshName").get<std::string>();
            object.layout = parseVertexLayout(objectJson.at("layout").get<std::string>());
            object.position = parseVec3(objectJson.at("position"), "objects.position");
            object.materialId = objectJson.at("materialId").get<std::string>();
            object.behavior = parseBehaviorType(objectJson.at("behavior").get<std::string>());
            object.behaviorSpeed = objectJson.value("behaviorSpeed", 0.0f);
            object.behaviorAxis = parseVec3(objectJson.at("behaviorAxis"), "objects.behaviorAxis");
            object.behaviorAmplitude = objectJson.value("behaviorAmplitude", 0.0f);
            definition.objects.push_back(object);
        }

        return definition;
    }

    void ensureLoaded()
    {
        if (gLoaded)
        {
            return;
        }

        std::ifstream file = openAssetFile("assets/scenes/scene_config.json");
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open assets/scenes/scene_config.json");
        }

        nlohmann::json root;
        file >> root;

        gSceneRegistry.clear();
        gSceneCycle.clear();
        gSceneDefinitions.clear();

        for (const nlohmann::json &entryJson : root.at("registry"))
        {
            const SceneId id = parseSceneId(entryJson.at("id").get<std::string>());
            const std::string filePath = entryJson.at("file").get<std::string>();

            gSceneRegistry.push_back({id, filePath});
            gSceneDefinitions[static_cast<int>(id)] = parseSceneDefinition(filePath);
        }

        for (const nlohmann::json &cycleJson : root.at("cycle"))
        {
            SceneCycleEntry entry;
            entry.id = parseSceneId(cycleJson.at("id").get<std::string>());
            entry.durationSeconds = cycleJson.at("durationSeconds").get<float>();
            gSceneCycle.push_back(entry);
        }

        gLoaded = true;
    }
} // namespace

const std::vector<SceneRegistryEntry> &getSceneRegistry()
{
    ensureLoaded();
    return gSceneRegistry;
}

const std::vector<SceneCycleEntry> &getDefaultSceneCycle()
{
    ensureLoaded();
    return gSceneCycle;
}

bool tryCreateSceneDefinition(SceneId id, SceneDefinition &outDefinition)
{
    ensureLoaded();

    const auto it = gSceneDefinitions.find(static_cast<int>(id));
    if (it == gSceneDefinitions.end())
    {
        return false;
    }

    outDefinition = it->second;
    return true;
}
