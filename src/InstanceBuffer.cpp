#include "InstanceBuffer.h"

#include <glm/glm.hpp>

InstanceBuffer::InstanceBuffer() : vao(0), instanceVbo(0), activeCount(0)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &instanceVbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_COPY_WRITE_BUFFER, instanceVbo);
    glBufferData(GL_COPY_WRITE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // Set up attribute pointers for mat4 (4 consecutive vec4 attributes)
    // Attributes 2-5 are reserved for instance transforms
    for (int i = 0; i < 4; ++i)
    {
        glEnableVertexAttribArray(2 + i);
        glVertexAttribPointer(
            2 + i,                         // attribute location
            4,                             // components per attribute (vec4)
            GL_FLOAT,                      // type
            GL_FALSE,                      // normalized
            sizeof(glm::mat4),             // stride (size of one mat4)
            (void*)(i * sizeof(glm::vec4)) // offset within mat4
        );
        glVertexAttribDivisor(2 + i, 1); // advance every instance
    }

    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    glBindVertexArray(0);
}

InstanceBuffer::~InstanceBuffer()
{
    if (instanceVbo != 0)
        glDeleteBuffers(1, &instanceVbo);
    if (vao != 0)
        glDeleteVertexArrays(1, &vao);
}

std::size_t InstanceBuffer::addInstance(const glm::mat4& modelMatrix)
{
    std::size_t index = matrices.size();
    matrices.push_back(modelMatrix);
    activeCount++;
    uploadMatricesToGpu();
    return index;
}

void InstanceBuffer::updateInstance(std::size_t      index,
                                    const glm::mat4& modelMatrix)
{
    if (index < matrices.size())
    {
        matrices[index] = modelMatrix;
        uploadMatricesToGpu();
    }
}

void InstanceBuffer::removeInstance(std::size_t index)
{
    if (index < matrices.size())
    {
        // Mark as removed by setting to identity (no-op transform)
        matrices[index] = glm::mat4(1.0f);
        if (activeCount > 0)
            activeCount--;
    }
}

void InstanceBuffer::bind() const
{
    glBindVertexArray(vao);
}

std::size_t InstanceBuffer::getInstanceCount() const
{
    return activeCount;
}

std::size_t InstanceBuffer::getCapacity() const
{
    return matrices.size();
}

void InstanceBuffer::uploadMatricesToGpu()
{
    glBindBuffer(GL_COPY_WRITE_BUFFER, instanceVbo);
    glBufferData(GL_COPY_WRITE_BUFFER, matrices.size() * sizeof(glm::mat4),
                 matrices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}
