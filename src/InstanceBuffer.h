#ifndef INSTANCEBUFFER_H
#define INSTANCEBUFFER_H

#include <cstddef>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

class InstanceBuffer
{
    public:
        enum class InstanceAttributeLocation : GLuint
        {
            BasePositionRotationAngle = 2,
            BaseScaleBehaviorType     = 3,
            BaseRotationAxisSpeed     = 4,
            BehaviorAxisAmplitude     = 5,
            Color                     = 6,
        };

        struct InstanceData
        {
                glm::vec3 basePosition;
                float     baseRotationAngle;
                glm::vec3 baseScale;
                float     behaviorType;
                glm::vec3 baseRotationAxis;
                float     behaviorSpeed;
                glm::vec3 behaviorAxis;
                float     behaviorAmplitude;
                glm::vec4 instanceColor = glm::vec4(1.0f);
        };

        struct InstanceGpuData
        {
                glm::vec4 basePositionRotationAngle;
                glm::vec4 baseScaleBehaviorType;
                glm::vec4 baseRotationAxisSpeed;
                glm::vec4 behaviorAxisAmplitude;
        };

        InstanceBuffer();
        ~InstanceBuffer();

        std::size_t addInstance(const InstanceData& instanceData);

        void attachToBoundVao() const;

    private:
        unsigned int instanceVbo;
        unsigned int colorVbo;

        std::vector<InstanceGpuData> instanceGpuData;
        std::vector<glm::vec4>       colors;

        void uploadInstanceDataToGpu();
        void uploadColorsToGpu();
};

#endif // INSTANCEBUFFER_H
