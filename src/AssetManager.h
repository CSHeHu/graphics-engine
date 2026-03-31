#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Shader;

/**
 * @brief Lazy-loading cache for meshes and shader programs.
 */
class AssetManager
{
public:
    AssetManager() = default;
    ~AssetManager() = default;

    /** @brief Get mesh vertex buffer data by mesh file name. */
    const std::vector<float> &getMeshVertices(const std::string &meshName);
    /** @brief Get vertex count for a mesh. */
    std::size_t getMeshVertexCount(const std::string &meshName);

    /** @brief Get or create a shader program for the provided shader paths. */
    std::shared_ptr<Shader> getShader(const std::string &vertexPath,
                                      const std::string &fragmentPath,
                                      const std::string &geometryPath = "");

private:
    /** @brief Parse OBJ mesh data into position+normal packed vertices. */
    std::vector<float> loadObjPositionNormal(const std::string &meshName) const;

    /** Mesh cache keyed by mesh file name. */
    std::unordered_map<std::string, std::vector<float>> meshCache;
    /** Shader cache keyed by combined shader path signature. */
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaderCache;
};

#endif // ASSETMANAGER_H