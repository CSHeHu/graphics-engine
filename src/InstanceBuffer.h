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
            ModelColumn0 = 2,
            ModelColumn1 = 3,
            ModelColumn2 = 4,
            ModelColumn3 = 5,
            Color        = 6,
        };
        InstanceBuffer();
        ~InstanceBuffer();

        std::size_t
             addInstance(const glm::mat4& modelMatrix,
                         const glm::vec4& instanceColor = glm::vec4(1.0f));
        void updateInstance(std::size_t index, const glm::mat4& modelMatrix);

        /** Prepare a compact draw buffer from a subset of instance indices. */
        void prepareDraw(const std::vector<std::size_t>& instanceIndices);

        void        attachToBoundVao() const;
        std::size_t getPreparedInstanceCount() const;

    private:
        unsigned int instanceVbo;
        unsigned int colorVbo;
        std::size_t  preparedCount;

        std::vector<glm::mat4> matrices;
        std::vector<glm::vec4> colors;
        std::vector<glm::mat4> drawMatrices;
        std::vector<glm::vec4> drawColors;

        void uploadMatricesToGpu();
        void uploadColorsToGpu();
        void uploadInstanceToGpu(std::size_t index);
        void uploadDrawBuffersToGpu();
};

#endif // INSTANCEBUFFER_H
