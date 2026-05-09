#ifndef INSTANCEBUFFER_H
#define INSTANCEBUFFER_H

#include <cstddef>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

class InstanceBuffer
{
    public:
        InstanceBuffer();
        ~InstanceBuffer();

        std::size_t addInstance(const glm::mat4& modelMatrix);
        void updateInstance(std::size_t index, const glm::mat4& modelMatrix);
        void removeInstance(std::size_t index);

        void        attachToBoundVao() const;
        void        bindBuffer() const;
        std::size_t getInstanceCount() const;
        std::size_t getCapacity() const;

    private:
        unsigned int           instanceVbo;
        std::vector<glm::mat4> matrices;
        std::size_t            activeCount;

        void uploadMatricesToGpu();
};

#endif // INSTANCEBUFFER_H
