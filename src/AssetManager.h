#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <cstddef>
#include <cstdint>
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
        /**
         * @brief Construct an asset manager with a mesh base path.
         * @param meshesPath Base directory used to resolve mesh file names.
         */
        explicit AssetManager(std::string meshesPath);
        ~AssetManager() = default;

        /**
         * @brief Load mesh data into cache if needed and return it.
         * @param meshName Mesh file name as referenced by scene content.
         * @return Cached mesh data for the requested mesh.
         */
        const MeshData& loadMesh(const std::string& meshName);
        /**
         * @brief Get vertex buffer data for an already loaded mesh.
         * @param meshName Mesh file name used as cache key.
         * @return Vertex data array in position-normal interleaved format.
         */
        const std::vector<float>&
        getMeshVertices(const std::string& meshName) const;
        /**
         * @brief Get number of vertices for an already loaded mesh.
         * @param meshName Mesh file name used as cache key.
         * @return Vertex count derived from cached interleaved vertex data.
         */
        std::size_t getMeshVertexCount(const std::string& meshName) const;
        /**
         * @brief Get index buffer data for an already loaded mesh.
         * @param meshName Mesh file name used as cache key.
         * @return Index data array used for indexed drawing.
         */
        const std::vector<unsigned int>&
        getMeshIndices(const std::string& meshName) const;
        /**
         * @brief Get number of indices for an already loaded mesh.
         * @param meshName Mesh file name used as cache key.
         * @return Total index count in cached index buffer.
         */
        std::size_t getMeshIndexCount(const std::string& meshName) const;

        /**
         * @brief Load shader program into cache if needed and return it.
         * @param vertexPath Vertex shader source path.
         * @param fragmentPath Fragment shader source path.
         * @param geometryPath Optional geometry shader source path.
         * @return Shared shader instance from cache.
         */
        std::shared_ptr<Shader>
        loadShader(const std::string& vertexPath,
                   const std::string& fragmentPath,
                   const std::string& geometryPath = "");
        /**
         * @brief Retrieve a previously loaded shader from cache.
         * @param vertexPath Vertex shader source path.
         * @param fragmentPath Fragment shader source path.
         * @param geometryPath Optional geometry shader source path.
         * @return Shared shader instance matching the exact shader key.
         */
        std::shared_ptr<Shader>
        getShader(const std::string& vertexPath,
                  const std::string& fragmentPath,
                  const std::string& geometryPath = "") const;

    private:
        struct Vec3
        {
                float x;
                float y;
                float z;
        };

        struct FaceVertex
        {
                int positionIndex;
                int normalIndex;
        };

        /**
         * @brief Pack OBJ position/normal indices into a single map key.
         * @param positionIndex Resolved OBJ position index.
         * @param normalIndex Resolved OBJ normal index.
         * @return Combined 64-bit key suitable for unordered_map lookup.
         */
        static uint64_t packVertexKey(int positionIndex, int normalIndex);
        /**
         * @brief Resolve positive/negative OBJ indices into absolute index
         * form.
         * @param index OBJ index token value.
         * @param size Maximum element count for the target array.
         * @return Resolved one-based index or zero if unresolved.
         */
        static int resolveIndex(int index, int size);
        /**
         * @brief Parse one OBJ face token into position/normal indices.
         * @param token Face token such as v//n or v/t/n.
         * @return Parsed position and normal indices.
         */
        static FaceVertex parseFaceToken(const std::string& token);

        /**
         * @brief Get cached mesh data or throw if mesh is not loaded.
         * @param meshName Mesh file name used as cache key.
         * @return Const reference to cached mesh data.
         */
        const MeshData& getLoadedMeshOrThrow(const std::string& meshName) const;

        /**
         * @brief Parse OBJ mesh file into packed position-normal vertex/index
         * data.
         * @param meshName Mesh file name to open relative to configured mesh
         * path.
         * @return Parsed mesh data ready for GPU buffer creation.
         */
        MeshData loadObjPositionNormal(const std::string& meshName) const;

        std::string meshesPath;

        /** Mesh cache keyed by mesh file name. */
        std::unordered_map<std::string, MeshData> meshCache;
        /** Shader cache keyed by combined shader path signature. */
        std::unordered_map<std::string, std::shared_ptr<Shader>> shaderCache;
};

#endif // ASSETMANAGER_H
