#ifndef MESHDATA_H
#define MESHDATA_H

#include <cstddef>
#include <vector>

namespace MeshData
{
    const std::vector<float> &getCubeVertices();
    const std::vector<float> &getGroundVertices();
    std::size_t getCubeVertexCount();
    std::size_t getGroundVertexCount();
} // namespace MeshData

#endif