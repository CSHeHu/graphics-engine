#include "AssetManager.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include <glm.hpp>

#include "Shader.h"

int AssetManager::resolveObjIndex(int index, int size)
{
    // OBJ indices are 1-based. Positive values are absolute, negative values
    // are relative to the end of the array (e.g., -1 = last element).
    if (index > 0)
    {
        return index;
    }
    if (index < 0)
    {
        return size + index + 1;
    }
    return 0;
}

AssetManager::FaceVertex
AssetManager::parseObjFaceToken(const std::string& token)
{
    // OBJ face token formats: v, v/t, v//n, v/t/n
    // We extract position and normal indices; texture coordinates are ignored.

    // Find the first '/' delimiter (if it exists)
    size_t slash1 = token.find('/');

    // Extract position index (always present, before first '/')
    int posIndex = std::stoi(
        token.substr(0, slash1 == std::string::npos ? token.length() : slash1));

    // If no '/', format is simply "v" (position only)
    if (slash1 == std::string::npos)
    {
        return FaceVertex{posIndex, 0};
    }

    // Find the second '/' delimiter (if it exists)
    size_t slash2 = token.find('/', slash1 + 1);

    // If no second '/', format is "v/t" (position and texture, no normal)
    if (slash2 == std::string::npos)
    {
        return FaceVertex{posIndex, 0};
    }

    // Extract normal index (after second '/')
    // Format is either "v/t/n" or "v//n"
    std::string normalStr = token.substr(slash2 + 1);

    // If empty, format was "v/t/" or "v//" with no normal provided
    if (normalStr.empty())
    {
        return FaceVertex{posIndex, 0};
    }

    int normIndex = std::stoi(normalStr);
    return FaceVertex{posIndex, normIndex};
}

const MeshData&
AssetManager::requireCachedMeshData(const std::string& meshName) const
{
    auto it = meshCache.find(meshName);
    if (it == meshCache.end())
    {
        throw std::runtime_error("Mesh not loaded: " + meshName);
    }
    return *it->second;
}

AssetManager::AssetManager(std::string meshesPathValue)
    : meshesPath(std::move(meshesPathValue))
{
}

const MeshData& AssetManager::loadMeshData(const std::string& meshName)
{
    auto it = meshCache.find(meshName);
    if (it != meshCache.end())
    {
        return *it->second;
    }

    auto inserted = meshCache.emplace(
        meshName,
        std::make_shared<MeshData>(parseObjMeshDataPositionNormal(meshName)));
    return *inserted.first->second;
}

const std::vector<float>&
AssetManager::getMeshVertexBuffer(const std::string& meshName) const
{
    return requireCachedMeshData(meshName).vertices;
}

std::size_t AssetManager::getMeshVertexCount(const std::string& meshName) const
{
    const auto&           verts           = getMeshVertexBuffer(meshName);
    constexpr std::size_t floatsPerVertex = 6; // x,y,z + nx,ny,nz
    return verts.size() / floatsPerVertex;
}

const std::vector<unsigned int>&
AssetManager::getMeshIndexBuffer(const std::string& meshName) const
{
    return requireCachedMeshData(meshName).indices;
}

std::size_t AssetManager::getMeshIndexCount(const std::string& meshName) const
{
    const auto& indices = getMeshIndexBuffer(meshName);
    return indices.size();
}

std::shared_ptr<Mix_Music> AssetManager::loadAudio(const std::string& file)
{
    auto it = audioCache.find(file);
    if (it != audioCache.end())
    {
        return it->second;
    }

    Mix_Music* music = Mix_LoadMUS(file.c_str());
    if (music == nullptr)
    {
        throw std::runtime_error("Failed to load audio: " + file);
    }

    std::shared_ptr<Mix_Music> cachedMusic(music, Mix_FreeMusic);
    audioCache.emplace(file, cachedMusic);
    return cachedMusic;
}

std::shared_ptr<Shader>
AssetManager::loadShader(const std::string& vertexPath,
                         const std::string& fragmentPath,
                         const std::string& geometryPath)
{
    const ShaderKey key{vertexPath, fragmentPath, geometryPath};

    auto it = shaderCache.find(key);
    if (it != shaderCache.end())
    {
        return it->second;
    }

    const char* geometryPathCStr =
        geometryPath.empty() ? nullptr : geometryPath.c_str();
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(
        vertexPath.c_str(), fragmentPath.c_str(), geometryPathCStr);
    shaderCache.emplace(key, shader);
    return shader;
}

std::shared_ptr<Shader>
AssetManager::getShader(const std::string& vertexPath,
                        const std::string& fragmentPath,
                        const std::string& geometryPath) const
{
    const ShaderKey key{vertexPath, fragmentPath, geometryPath};
    auto            it = shaderCache.find(key);
    if (it == shaderCache.end())
    {
        throw std::runtime_error("Shader not loaded: " + vertexPath + " | " +
                                 fragmentPath + " | " + geometryPath);
    }
    return it->second;
}

MeshData
AssetManager::parseObjMeshDataPositionNormal(const std::string& meshName) const
{
    const std::vector<std::string> candidatePaths = {
        meshesPath + "/" + meshName,
        "../" + meshesPath + "/" + meshName,
    };

    std::ifstream file;
    std::string   openedPath;
    for (const std::string& path : candidatePaths)
    {
        file.open(path);
        if (file.is_open())
        {
            openedPath = path;
            break;
        }
        file.clear();
    }

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open mesh file for " + meshName);
    }

    std::vector<glm::vec3> positions(1, {0.0f, 0.0f, 0.0f});
    std::vector<glm::vec3> normals(1, {0.0f, 0.0f, 0.0f});
    const glm::vec3        defaultNormal{0.0f, 1.0f, 0.0f};
    MeshData               mesh;

    struct VertexKey
    {
            int  p;
            int  n;
            bool operator<(VertexKey const& o) const
            {
                if (p != o.p)
                    return p < o.p;
                return n < o.n;
            }
    };

    std::map<VertexKey, unsigned int> vertexMap;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::istringstream lineStream(line);
        std::string        type;
        lineStream >> type;

        if (type == "v")
        {
            glm::vec3 p{};
            lineStream >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (type == "vn")
        {
            glm::vec3 n{};
            lineStream >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (type == "f")
        {
            std::vector<AssetManager::FaceVertex> face;
            std::string                           token;
            while (lineStream >> token)
            {
                face.push_back(AssetManager::parseObjFaceToken(token));
            }

            if (face.size() < 3)
            {
                continue;
            }

            for (size_t i = 1; i + 1 < face.size(); ++i)
            {
                const AssetManager::FaceVertex tri[3] = {face[0], face[i],
                                                         face[i + 1]};
                for (const AssetManager::FaceVertex& fv : tri)
                {
                    // Resolve OBJ indices (1-based or negative-relative) to
                    // absolute array indices.
                    const int pi = AssetManager::resolveObjIndex(
                        fv.positionIndex,
                        static_cast<int>(positions.size()) - 1);
                    const int ni = AssetManager::resolveObjIndex(
                        fv.normalIndex, static_cast<int>(normals.size()) - 1);

                    if (pi <= 0 || pi >= static_cast<int>(positions.size()))
                    {
                        throw std::runtime_error(
                            "Position index out of range while parsing " +
                            openedPath);
                    }

                    const int safeNormalIndex =
                        (ni > 0 && ni < static_cast<int>(normals.size())) ? ni
                                                                          : 0;

                    VertexKey vk{pi, safeNormalIndex};
                    auto      mapped = vertexMap.find(vk);
                    if (mapped == vertexMap.end())
                    {
                        const glm::vec3& p = positions[pi];
                        const glm::vec3& n = (safeNormalIndex > 0)
                                                 ? normals[safeNormalIndex]
                                                 : defaultNormal;

                        mesh.vertices.push_back(p.x);
                        mesh.vertices.push_back(p.y);
                        mesh.vertices.push_back(p.z);
                        mesh.vertices.push_back(n.x);
                        mesh.vertices.push_back(n.y);
                        mesh.vertices.push_back(n.z);

                        const unsigned int newIndex = static_cast<unsigned int>(
                            (mesh.vertices.size() / 6) - 1);
                        vertexMap.emplace(vk, newIndex);
                        mesh.indices.push_back(newIndex);
                    }
                    else
                    {
                        mesh.indices.push_back(mapped->second);
                    }
                }
            }
        }
    }

    if (mesh.vertices.empty() || mesh.indices.empty())
    {
        throw std::runtime_error("Mesh file contained no triangles: " +
                                 openedPath);
    }

    std::cout << "Loaded mesh: " << openedPath << " ("
              << mesh.vertices.size() / 6 << " unique vertices, "
              << mesh.indices.size() / 3 << " triangles)" << std::endl;
    return mesh;
}
