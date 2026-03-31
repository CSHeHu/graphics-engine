#ifndef OBJECT_H
#define OBJECT_H

#include <memory>
#include <string>
#include <vector>
#include <glm.hpp>
#include "Shader.h"
#include "TextureManager.h"
#include <glad/glad.h>

class Object
{
public:
    enum class VertexLayout
    {
        PositionUV,
        PositionNormal
    };

    /** @brief Construct renderable object from vertex data and initial transform. */
    Object(std::shared_ptr<Shader> shaderProgram,
           const std::vector<float> &vertices,
           const glm::vec3 &position,
           const glm::vec3 &scale,
           VertexLayout layout,
           const std::vector<std::string> &texturePaths = {});
    /** @brief Destroy OpenGL buffers owned by this object. */
    ~Object();

    /** @brief Get object world position. */
    glm::vec3 getPosition() const;
    /** @brief Set object world position. */
    void setPosition(glm::vec3 position);
    /** @brief Get object scale. */
    glm::vec3 getScale() const;
    /** @brief Set object scale. */
    void setScale(const glm::vec3 &scale);
    /** @brief Get rotation angle in radians. */
    float getRotationAngle() const;
    /** @brief Get normalized rotation axis. */
    glm::vec3 getRotationAxis() const;
    /** @brief Set absolute rotation. */
    void setRotation(float angleRadians, const glm::vec3 &axis);
    /** @brief Add incremental rotation around axis. */
    void rotate(float deltaAngleRadians, const glm::vec3 &axis);
    /** @brief Build model matrix from position, rotation and scale. */
    glm::mat4 getModelMatrix() const;
    /** @brief Get VAO handle. */
    unsigned int getVAO() const;
    /** @brief Get VBO handle. */
    unsigned int getVBO() const;

    std::shared_ptr<Shader> shader;
    TextureManager textureManager;

private:
    unsigned int VAO, VBO;
    glm::vec3 pos;
    glm::vec3 size;
    float rotationAngle;
    glm::vec3 rotationAxis;
};

#endif // OBJECT_H
