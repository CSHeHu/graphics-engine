#include "AssetManager.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "SceneDefinitions.h"
#include "Shader.h"

namespace
{
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

uint64_t packVertexKey(int positionIndex, int normalIndex)
{
    return (static_cast<uint64_t>(static_cast<uint32_t>(positionIndex)) << 32) |
           static_cast<uint32_t>(normalIndex);
}

int resolveIndex(int index, int size)
{
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

FaceVertex parseFaceToken(const std::string& token)
{
    FaceVertex fv{0, 0};

    int p = 0;
    int t = 0;
    int n = 0;
    if (std::sscanf(token.c_str(), "%d//%d", &p, &n) == 2)
    {
        fv.positionIndex = p;
        fv.normalIndex   = n;
        return fv;
    }
    if (std::sscanf(token.c_str(), "%d/%d/%d", &p, &t, &n) == 3)
    {
        (void)t;
        fv.positionIndex = p;
        fv.normalIndex   = n;
        return fv;
    }
    if (std::sscanf(token.c_str(), "%d", &p) == 1)
    {
        fv.positionIndex = p;
        fv.normalIndex   = 0;
        return fv;
    }

    throw std::runtime_error("Unsupported face token: " + token);
}
} // namespace

const std::vector<float>&
AssetManager::getMeshVertices(const std::string& meshName)
{
    auto it = meshCache.find(meshName);
    if (it != meshCache.end())
    {
        return it->second.vertices;
    }

    auto inserted =
        meshCache.emplace(meshName, loadObjPositionNormal(meshName));
    return inserted.first->second.vertices;
}

std::size_t AssetManager::getMeshVertexCount(const std::string& meshName)
{
    return getMeshIndexCount(meshName);
}

const std::vector<unsigned int>&
AssetManager::getMeshIndices(const std::string& meshName)
{
    auto it = meshCache.find(meshName);
    if (it != meshCache.end())
    {
        return it->second.indices;
    }

    auto inserted =
        meshCache.emplace(meshName, loadObjPositionNormal(meshName));
    return inserted.first->second.indices;
}

std::size_t AssetManager::getMeshIndexCount(const std::string& meshName)
{
    const std::vector<unsigned int>& indices = getMeshIndices(meshName);
    return indices.size();
}

std::shared_ptr<Shader> AssetManager::getShader(const std::string& vertexPath,
                                                const std::string& fragmentPath,
                                                const std::string& geometryPath)
{
    const std::string key =
        vertexPath + "|" + fragmentPath + "|" + geometryPath;

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

MeshData AssetManager::loadObjPositionNormal(const std::string& meshName) const
{
    const std::string& meshesPath =
        SceneDefinitions::getRuntimeConfig().assets.meshesPath;
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

    std::vector<Vec3>                          positions(1, {0.0f, 0.0f, 0.0f});
    std::vector<Vec3>                          normals(1, {0.0f, 0.0f, 0.0f});
    const Vec3                                 defaultNormal{0.0f, 1.0f, 0.0f};
    MeshData                                   mesh;
    std::unordered_map<uint64_t, unsigned int> vertexMap;

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
            Vec3 p{};
            lineStream >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (type == "vn")
        {
            Vec3 n{};
            lineStream >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (type == "f")
        {
            std::vector<FaceVertex> face;
            std::string             token;
            while (lineStream >> token)
            {
                face.push_back(parseFaceToken(token));
            }

            if (face.size() < 3)
            {
                continue;
            }

            for (size_t i = 1; i + 1 < face.size(); ++i)
            {
                const FaceVertex tri[3] = {face[0], face[i], face[i + 1]};
                for (const FaceVertex& fv : tri)
                {
                    const int pi =
                        resolveIndex(fv.positionIndex,
                                     static_cast<int>(positions.size()) - 1);
                    const int ni = resolveIndex(
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
                    const uint64_t key    = packVertexKey(pi, safeNormalIndex);
                    auto           mapped = vertexMap.find(key);
                    if (mapped == vertexMap.end())
                    {
                        const Vec3& p = positions[pi];
                        const Vec3& n = (safeNormalIndex > 0)
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
                        vertexMap.emplace(key, newIndex);
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