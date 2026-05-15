#ifndef OBJECT_H
#define OBJECT_H

#include <glm.hpp>

class Object
{
    public:
        /** @brief Construct lightweight runtime object transform. */
        Object(const glm::vec3& position, const glm::vec3& scale);
        /** @brief Get object world position. */
        const glm::vec3& getPosition() const;
        /** @brief Set object world position. */
        void setPosition(const glm::vec3& position);
        /** @brief Get rotation angle in radians. */
        float getRotationAngle() const;
        /** @brief Get normalized rotation axis. */
        const glm::vec3& getRotationAxis() const;
        /** @brief Set absolute rotation. */
        void setRotation(float angleRadians, const glm::vec3& axis);
        /** @brief Add incremental rotation around axis. */
        void rotate(float deltaAngleRadians, const glm::vec3& axis);
        /** @brief Build model matrix from position, rotation and scale. */
        const glm::mat4 getModelMatrix() const;

    private:
        glm::vec3 pos;
        glm::vec3 size;
        float     rotationAngle;
        glm::vec3 rotationAxis;
};

#endif // OBJECT_H
