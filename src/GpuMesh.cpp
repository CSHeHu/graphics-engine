#include "GpuMesh.h"

#include <stdexcept>

GpuMesh::GpuMesh(const std::vector<float>&        vertices,
                 const std::vector<unsigned int>& indices, VertexLayout layout)
    : vao(0), vbo(0), ebo(0), indexCount(indices.size())
{
    if (layout != VertexLayout::PositionNormal)
    {
        throw std::runtime_error("Unsupported vertex layout for GpuMesh");
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

GpuMesh::~GpuMesh()
{
    if (vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
    }
    if (vbo != 0)
    {
        glDeleteBuffers(1, &vbo);
    }
    if (ebo != 0)
    {
        glDeleteBuffers(1, &ebo);
    }
}

void GpuMesh::bind() const
{
    glBindVertexArray(vao);
}

std::size_t GpuMesh::getIndexCount() const
{
    return indexCount;
}