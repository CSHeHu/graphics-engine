#include "InstanceBuffer.h"

#include <glm/glm.hpp>

static constexpr std::size_t kModelMatrixColumnCount = 4;

InstanceBuffer::InstanceBuffer() : instanceVbo(0), colorVbo(0), activeCount(0)
{
    glGenBuffers(1, &instanceVbo);
    glGenBuffers(1, &colorVbo);
}

InstanceBuffer::~InstanceBuffer()
{
    if (instanceVbo != 0)
        glDeleteBuffers(1, &instanceVbo);
    if (colorVbo != 0)
        glDeleteBuffers(1, &colorVbo);
}

std::size_t InstanceBuffer::addInstance(const glm::mat4& modelMatrix,
                                        const glm::vec4& instanceColor)
{
    const std::size_t index = matrices.size();
    matrices.push_back(modelMatrix);
    colors.push_back(instanceColor);
    ++activeCount;
    uploadMatricesToGpu();
    uploadColorsToGpu();
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
    for (std::size_t column = 0; column < kModelMatrixColumnCount; ++column)
    {
        const GLuint attributeLocation =
            static_cast<GLuint>(InstanceAttributeLocation::ModelColumn0) +
            static_cast<GLuint>(column);
        glEnableVertexAttribArray(attributeLocation);
        glVertexAttribPointer(
            attributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
            reinterpret_cast<void*>(column * sizeof(glm::vec4)));
        glVertexAttribDivisor(attributeLocation, 1);
    }
    glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
    const GLuint colorLocation =
        static_cast<GLuint>(InstanceAttributeLocation::Color);
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 4, GL_FLOAT, GL_FALSE,
                          sizeof(glm::vec4), nullptr);
    glVertexAttribDivisor(colorLocation, 1);
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

void InstanceBuffer::uploadColorsToGpu()
{
    glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4),
                 colors.data(), GL_DYNAMIC_DRAW);
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
