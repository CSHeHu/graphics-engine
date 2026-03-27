#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Shader;

class AssetManager
{
public:
    AssetManager() = default;
    ~AssetManager() = default;

    const std::vector<float> &getMeshVertices(const std::string &meshName);
    std::size_t getMeshVertexCount(const std::string &meshName);

    std::shared_ptr<Shader> getShader(const std::string &vertexPath,
                                      const std::string &fragmentPath,
                                      const std::string &geometryPath = "");

private:
    std::vector<float> loadObjPositionNormal(const std::string &meshName) const;

    std::unordered_map<std::string, std::vector<float>> meshCache;
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaderCache;
};

#endif // ASSETMANAGER_H