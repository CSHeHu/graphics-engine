#include "InstanceBuffer.h"

#include <glm/glm.hpp>

InstanceBuffer::InstanceBuffer() : instanceVbo(0), colorVbo(0)
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

std::size_t InstanceBuffer::addInstance(const InstanceData& instanceData)
{
    const std::size_t index = instanceGpuData.size();
    InstanceGpuData   gpuData;
    gpuData.basePositionRotationAngle =
        glm::vec4(instanceData.basePosition, instanceData.baseRotationAngle);
    gpuData.baseScaleBehaviorType =
        glm::vec4(instanceData.baseScale, instanceData.behaviorType);
    gpuData.baseRotationAxisSpeed =
        glm::vec4(instanceData.baseRotationAxis, instanceData.behaviorSpeed);
    gpuData.behaviorAxisAmplitude =
        glm::vec4(instanceData.behaviorAxis, instanceData.behaviorAmplitude);
    instanceGpuData.push_back(gpuData);
    colors.push_back(instanceData.instanceColor);
    return index;
}

void InstanceBuffer::setDrawInstances(const std::vector<std::size_t>& indices)
{
    drawInstanceGpuData.clear();
    drawColors.clear();

    drawInstanceGpuData.reserve(indices.size());
    drawColors.reserve(indices.size());
    for (const std::size_t index : indices)
    {
        if (index >= instanceGpuData.size() || index >= colors.size())
        {
            continue;
        }
        drawInstanceGpuData.push_back(instanceGpuData[index]);
        drawColors.push_back(colors[index]);
    }

    uploadInstanceDataToGpu();
    uploadColorsToGpu();
}

void InstanceBuffer::attachToBoundVao() const
{
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
    glEnableVertexAttribArray(static_cast<GLuint>(
        InstanceAttributeLocation::BasePositionRotationAngle));
    glVertexAttribPointer(
        static_cast<GLuint>(
            InstanceAttributeLocation::BasePositionRotationAngle),
        4, GL_FLOAT, GL_FALSE, sizeof(InstanceGpuData),
        reinterpret_cast<void*>(
            offsetof(InstanceGpuData, basePositionRotationAngle)));
    glVertexAttribDivisor(
        static_cast<GLuint>(
            InstanceAttributeLocation::BasePositionRotationAngle),
        1);

    glEnableVertexAttribArray(
        static_cast<GLuint>(InstanceAttributeLocation::BaseScaleBehaviorType));
    glVertexAttribPointer(
        static_cast<GLuint>(InstanceAttributeLocation::BaseScaleBehaviorType),
        4, GL_FLOAT, GL_FALSE, sizeof(InstanceGpuData),
        reinterpret_cast<void*>(
            offsetof(InstanceGpuData, baseScaleBehaviorType)));
    glVertexAttribDivisor(
        static_cast<GLuint>(InstanceAttributeLocation::BaseScaleBehaviorType),
        1);

    glEnableVertexAttribArray(
        static_cast<GLuint>(InstanceAttributeLocation::BaseRotationAxisSpeed));
    glVertexAttribPointer(
        static_cast<GLuint>(InstanceAttributeLocation::BaseRotationAxisSpeed),
        4, GL_FLOAT, GL_FALSE, sizeof(InstanceGpuData),
        reinterpret_cast<void*>(
            offsetof(InstanceGpuData, baseRotationAxisSpeed)));
    glVertexAttribDivisor(
        static_cast<GLuint>(InstanceAttributeLocation::BaseRotationAxisSpeed),
        1);

    glEnableVertexAttribArray(
        static_cast<GLuint>(InstanceAttributeLocation::BehaviorAxisAmplitude));
    glVertexAttribPointer(
        static_cast<GLuint>(InstanceAttributeLocation::BehaviorAxisAmplitude),
        4, GL_FLOAT, GL_FALSE, sizeof(InstanceGpuData),
        reinterpret_cast<void*>(
            offsetof(InstanceGpuData, behaviorAxisAmplitude)));
    glVertexAttribDivisor(
        static_cast<GLuint>(InstanceAttributeLocation::BehaviorAxisAmplitude),
        1);

    glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
    const GLuint colorLocation =
        static_cast<GLuint>(InstanceAttributeLocation::Color);
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 4, GL_FLOAT, GL_FALSE,
                          sizeof(glm::vec4), nullptr);
    glVertexAttribDivisor(colorLocation, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InstanceBuffer::uploadInstanceDataToGpu()
{
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
    glBufferData(
        GL_ARRAY_BUFFER, drawInstanceGpuData.size() * sizeof(InstanceGpuData),
        drawInstanceGpuData.empty() ? nullptr : drawInstanceGpuData.data(),
        GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InstanceBuffer::uploadColorsToGpu()
{
    glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
    glBufferData(GL_ARRAY_BUFFER, drawColors.size() * sizeof(glm::vec4),
                 drawColors.empty() ? nullptr : drawColors.data(),
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
