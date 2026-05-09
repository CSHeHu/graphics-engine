#ifndef INSTANCEBUFFER_H
#define INSTANCEBUFFER_H

#include <cstddef>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

/**
 * InstanceBuffer manages GPU storage for per-instance transform matrices.
 * One InstanceBuffer per unique mesh; stores up to N instances of that mesh.
 * Instances can be added, updated, and removed dynamically.
 */
class InstanceBuffer
{
    public:
        InstanceBuffer();
        ~InstanceBuffer();

        /**
         * Add a new instance with the given model matrix.
         * @return Index of the new instance (used for later updates/removal)
         */
        std::size_t addInstance(const glm::mat4& modelMatrix);

        /**
         * Update an existing instance's model matrix.
         * @param index Instance index (returned by addInstance)
         * @param modelMatrix New model matrix
         */
        void updateInstance(std::size_t index, const glm::mat4& modelMatrix);

        /**
         * Remove an instance by marking it as unused.
         * (For now, we don't defragment; removal sets a placeholder.)
         */
        void removeInstance(std::size_t index);

        /**
         * Bind this instance buffer's VAO for rendering.
         * Must be called after binding the GpuMesh's VAO.
         */
        void bind() const;

        /**
         * Return the number of active instances.
         */
        std::size_t getInstanceCount() const;

        /**
         * Return the total capacity (including removed instances).
         */
        std::size_t getCapacity() const;

    private:
        unsigned int vao; // VAO to hold instance VBO layout
        unsigned int
            instanceVbo; // VBO storing mat4 transforms (16 floats per instance)
        std::vector<glm::mat4> matrices;    // CPU-side copy for updates
        std::size_t            activeCount; // Number of non-removed instances

        void uploadMatricesToGpu();
};

#endif // INSTANCEBUFFER_H
