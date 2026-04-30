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
        explicit AssetManager(std::string meshesPath);
        ~AssetManager() = default;

        /** @brief Load and return mesh data for `meshName`. This explicitly
         *  loads (and caches) the mesh; callers should call this before using
         *  non-mutating getters. */
        const MeshData& loadMesh(const std::string& meshName);
        /** @brief Get vertex buffer data for a mesh. Requires the mesh to be
         *  already loaded via `loadMesh`. Throws if not loaded. */
        const std::vector<float>&
        getMeshVertices(const std::string& meshName) const;
        /** @brief Get vertex count (number of vertices). Requires mesh loaded.
         */
        std::size_t getMeshVertexCount(const std::string& meshName) const;
        /** @brief Get index buffer data for a mesh. Requires the mesh to be
         *  already loaded via `loadMesh`. Throws if not loaded. */
        const std::vector<unsigned int>&
        getMeshIndices(const std::string& meshName) const;
        /** @brief Get index count for a mesh. Requires mesh loaded. */
        std::size_t getMeshIndexCount(const std::string& meshName) const;

        /** @brief Load and cache a shader program for the provided shader paths
         *  and return it. */
        std::shared_ptr<Shader>
        loadShader(const std::string& vertexPath,
                   const std::string& fragmentPath,
                   const std::string& geometryPath = "");
        /** @brief Retrieve a previously loaded shader. Throws if not loaded.
         */
        std::shared_ptr<Shader>
        getShader(const std::string& vertexPath,
                  const std::string& fragmentPath,
                  const std::string& geometryPath = "") const;

    private:
        /** @brief Parse OBJ mesh data into position+normal packed vertices. */
        MeshData loadObjPositionNormal(const std::string& meshName) const;

        std::string meshesPath;

        /** Mesh cache keyed by mesh file name. */
        std::unordered_map<std::string, MeshData> meshCache;
        /** Shader cache keyed by combined shader path signature. */
        std::unordered_map<std::string, std::shared_ptr<Shader>> shaderCache;
};

#endif // ASSETMANAGER_H
