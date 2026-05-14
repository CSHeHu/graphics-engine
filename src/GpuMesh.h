#ifndef GPUMESH_H
#define GPUMESH_H

#include <cstddef>
#include <vector>

#include <glad/glad.h>

#include "VertexLayout.h"

class GpuMesh
{
    public:
        GpuMesh(const std::vector<float>&        vertices,
                const std::vector<unsigned int>& indices,
                VertexLayout layout = VertexLayout::PositionNormal);
        ~GpuMesh();

        void        bind() const;
        std::size_t getIndexCount() const;

    private:
        unsigned int vao;
        unsigned int vbo;
        unsigned int ebo;
        std::size_t  indexCount;
};

#endif // GPUMESH_H
