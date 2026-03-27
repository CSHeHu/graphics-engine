#ifndef OBJECT_H
#define OBJECT_H

#include <memory>
#include <string>
#include <vector>
#include <glm.hpp>
#include "Shader.h"
#include "TextureManager.h"
#include <glad/glad.h>

class Object {
public:
    enum class VertexLayout
    {
        PositionUV,
        PositionNormal
    };

    Object(std::shared_ptr<Shader> shaderProgram,
           const std::vector<float>& vertices,
           const glm::vec3& position,
           VertexLayout layout,
           const std::vector<std::string>& texturePaths = {});
    ~Object();

    glm::vec3 getPosition() const;
    void setPosition(glm::vec3 position);

    std::shared_ptr<Shader> shader;
    TextureManager textureManager;
    unsigned int VAO, VBO;

private:
    glm::vec3 pos;
};

#endif // OBJECT_H
