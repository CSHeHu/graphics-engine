#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL_mixer.h>

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

        /**
         * @brief Load and cache a music track by file path.
         * @param file Music file path.
         * @return Shared music instance from cache.
         */
        std::shared_ptr<Mix_Music> loadAudio(const std::string& file);

    private:
        struct FaceVertex
        {
                int positionIndex;
                int normalIndex;
        };

        struct ShaderKey
        {
                std::string vertexPath;
                std::string fragmentPath;
                std::string geometryPath;

                bool operator<(const ShaderKey& other) const
                {
                    if (vertexPath != other.vertexPath)
                    {
                        return vertexPath < other.vertexPath;
                    }
                    if (fragmentPath != other.fragmentPath)
                    {
                        return fragmentPath < other.fragmentPath;
                    }
                    return geometryPath < other.geometryPath;
                }
        };

        /**
         * @brief Resolve positive/negative OBJ indices into absolute index
         * form.
         * @param index OBJ index token value (1-based positive or negative
         * relative).
         * @param size Maximum element count for the target array.
         * @return Resolved absolute array index, or zero if invalid.
         */
        static int resolveObjIndex(int index, int size);

        /**
         * @brief Parse one OBJ face token into position/normal indices.
         * @param token Face token such as v//n or v/t/n (supports v, v/t, v//n,
         * v/t/n).
         * @return Parsed position and normal indices.
         */
        static FaceVertex parseObjFaceToken(const std::string& token);

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
        std::unordered_map<std::string, std::shared_ptr<MeshData>> meshCache;
        /** Shader cache keyed by explicit shader paths. */
        std::map<ShaderKey, std::shared_ptr<Shader>> shaderCache;

        std::unordered_map<std::string, std::shared_ptr<Mix_Music>> audioCache;
};

#endif // ASSETMANAGER_H
