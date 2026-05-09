#include "InstanceBuffer.h"

#include <glm/glm.hpp>

InstanceBuffer::InstanceBuffer() : instanceVbo(0), activeCount(0)
{
    glGenBuffers(1, &instanceVbo);
}

InstanceBuffer::~InstanceBuffer()
{
    if (instanceVbo != 0)
        glDeleteBuffers(1, &instanceVbo);
}

std::size_t InstanceBuffer::addInstance(const glm::mat4& modelMatrix)
{
    const std::size_t index = matrices.size();
    matrices.push_back(modelMatrix);
    ++activeCount;
    uploadMatricesToGpu();
    return index;
}

void InstanceBuffer::updateInstance(std::size_t      index,
                                    const glm::mat4& modelMatrix)
{
    if (index < matrices.size())
    {
        matrices[index] = modelMatrix;
        uploadInstanceToGpu(index);
    }
}

void InstanceBuffer::removeInstance(std::size_t index)
{
    if (index < matrices.size())
    {
        matrices[index] = glm::mat4(1.0f);
        if (activeCount > 0)
            --activeCount;
        uploadInstanceToGpu(index);
    }
}

void InstanceBuffer::attachToBoundVao() const
{
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
    for (int i = 0; i < 4; ++i)
    {
        glEnableVertexAttribArray(2 + i);
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                              reinterpret_cast<void*>(i * sizeof(glm::vec4)));
        glVertexAttribDivisor(2 + i, 1);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InstanceBuffer::bindBuffer() const
{
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
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
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
    glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(glm::mat4),
                 matrices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InstanceBuffer::uploadInstanceToGpu(std::size_t index)
{
    if (index >= matrices.size())
    {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
    glBufferSubData(GL_ARRAY_BUFFER,
                    static_cast<GLintptr>(index * sizeof(glm::mat4)),
                    sizeof(glm::mat4), &matrices[index]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
