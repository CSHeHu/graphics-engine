#ifndef TEXTMANAGER_H
#define TEXTMANAGER_H

#include <glm.hpp>
#include <memory>
#include <string>

class Shader;

class TextManager
{
public:
    TextManager();
    ~TextManager();

    bool init(const std::string &fontPath, const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
    void renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color);

private:
    struct GlyphData
    {
        unsigned int textureId;
        glm::ivec2 size;
        glm::ivec2 bearing;
        int advance;
    };

    std::unique_ptr<Shader> shader;
    unsigned int vao, vbo;
    GlyphData glyphs[128];
    bool initialized;
};

#endif // TEXTMANAGER_H
