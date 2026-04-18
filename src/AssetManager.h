#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Shader;

struct MeshData
{
    std::vector<float>        vertices;
    std::vector<unsigned int> indices;
};

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
    /** @brief Get mesh index buffer data by mesh file name. */
    const std::vector<unsigned int> &getMeshIndices(const std::string &meshName);
    /** @brief Get index count for a mesh. */
    std::size_t getMeshIndexCount(const std::string &meshName);

    /** @brief Get or create a shader program for the provided shader paths. */
    std::shared_ptr<Shader> getShader(const std::string &vertexPath,
                                      const std::string &fragmentPath,
                                      const std::string &geometryPath = "");

private:
    /** @brief Parse OBJ mesh data into position+normal packed vertices. */
    MeshData loadObjPositionNormal(const std::string &meshName) const;

    /** Mesh cache keyed by mesh file name. */
    std::unordered_map<std::string, MeshData> meshCache;
    /** Shader cache keyed by combined shader path signature. */
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaderCache;
};

#endif // ASSETMANAGER_H