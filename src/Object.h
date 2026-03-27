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

    Object(std::shared_ptr<Shader> shaderProgram,
           const std::vector<float> &vertices,
           const glm::vec3 &position,
           VertexLayout layout,
           const std::vector<std::string> &texturePaths = {});
    ~Object();

    glm::vec3 getPosition() const;
    void setPosition(glm::vec3 position);
    float getRotationAngle() const;
    glm::vec3 getRotationAxis() const;
    void setRotation(float angleRadians, const glm::vec3 &axis);
    void rotate(float deltaAngleRadians, const glm::vec3 &axis);
    glm::mat4 getModelMatrix() const;
    unsigned int getVAO() const;
    unsigned int getVBO() const;

    std::shared_ptr<Shader> shader;
    TextureManager textureManager;

private:
    unsigned int VAO, VBO;
    glm::vec3 pos;
    float rotationAngle;
    glm::vec3 rotationAxis;
};

#endif // OBJECT_H
