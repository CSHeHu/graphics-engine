#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm.hpp>

#include <string>

class Shader
{
public:
    unsigned int ID;
    /** @brief Build shader program from source file paths. */
    Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath = nullptr);
    /** @brief Bind this shader program for subsequent draw calls. */
    void use();
    /** @brief Set boolean uniform value. */
    void setBool(const std::string &name, bool value) const;
    /** @brief Set integer uniform value. */
    void setInt(const std::string &name, int value) const;
    /** @brief Set float uniform value. */
    void setFloat(const std::string &name, float value) const;
    /** @brief Set vec2 uniform value. */
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    /** @brief Set vec2 uniform value from components. */
    void setVec2(const std::string &name, float x, float y) const;
    /** @brief Set vec3 uniform value. */
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    /** @brief Set vec3 uniform value from components. */
    void setVec3(const std::string &name, float x, float y, float z) const;
    /** @brief Set vec4 uniform value. */
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    /** @brief Set vec4 uniform value from components. */
    void setVec4(const std::string &name, float x, float y, float z, float w) const;
    /** @brief Set mat2 uniform value. */
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    /** @brief Set mat3 uniform value. */
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    /** @brief Set mat4 uniform value. */
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    /** @brief Report compile/link errors for shader stages and programs. */
    void checkCompileErrors(GLuint shader, std::string type);
};

#endif
