#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <SDL_mixer.h>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "VertexLayout.h"

class Shader;
class GpuMesh;

/**
 * @brief Lazy-loading cache for meshes and shader programs.
 */
class AssetManager
{
    public:
        struct MeshData
        {
                std::vector<float>        vertices;
                std::vector<unsigned int> indices;
        };

        /**
         * @brief Construct an asset manager with a mesh base path.
         * @param meshesPath Base directory used to resolve mesh file names.
         */
        AssetManager(const std::string& meshesPath);
        ~AssetManager() = default;

        /**
         * @brief Load mesh data into cache if needed and return it.
         * @param meshName Mesh file name as referenced by scene content.
         * @return Cached mesh data for the requested mesh.
         */
        const MeshData& loadMeshData(const std::string& meshName);

        /**
         * @brief Load a mesh into GPU memory if needed and return the shared
         * handle.
         * @param meshName Mesh file name as referenced by scene content.
         * @return Cached GPU mesh for the requested mesh.
         */
        std::shared_ptr<GpuMesh> loadGpuMesh(const std::string& meshName,
                                             VertexLayout       layout);

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
         * @brief Load and cache a music track by file path.
         * @param file Music file path.
         * @return Shared music instance from cache.
         */
        std::shared_ptr<Mix_Music> loadAudio(const std::string& file);

    private:
        std::string meshesPath;

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

        /** Mesh cache keyed by mesh file name. */
        std::unordered_map<std::string, std::shared_ptr<MeshData>> meshCache;
        /** GPU mesh cache keyed by mesh file name. */
        std::unordered_map<std::string, std::shared_ptr<GpuMesh>> gpuMeshCache;
        /** Shader cache keyed by explicit shader paths. */
        std::map<ShaderKey, std::shared_ptr<Shader>> shaderCache;

        std::unordered_map<std::string, std::shared_ptr<Mix_Music>> audioCache;

        /**
         * @brief Resolve positive/negative OBJ indices into absolute index
         * form.
         * @param index OBJ index token value (1-based positive or negative
         * relative).
         * @param size Maximum element count for the target array.
         * @return Resolved absolute array index, or zero if invalid.
         */
        int resolveObjIndex(int index, int size) const;

        /**
         * @brief Parse one OBJ face token into position/normal indices.
         * @param token Face token such as v//n or v/t/n (supports v, v/t, v//n,
         * v/t/n).
         * @return Parsed position and normal indices.
         */
        FaceVertex parseObjFaceToken(const std::string& token) const;

        /**
         * @brief Get cached mesh data or throw if mesh is not loaded.
         * @param meshName Mesh file name used as cache key.
         * @return Const reference to cached mesh data.
         */
        const MeshData&
        requireCachedMeshData(const std::string& meshName) const;

        /**
         * @brief Parse OBJ mesh file into packed position-normal vertex/index
         * data.
         * @param meshName Mesh file name to open relative to configured mesh
         * path.
         * @return Parsed mesh data ready for GPU buffer creation.
         */
        MeshData
        parseObjMeshDataPositionNormal(const std::string& meshName) const;
};

#endif // ASSETMANAGER_H
