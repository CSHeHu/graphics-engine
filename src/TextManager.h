#ifndef TEXTMANAGER_H
#define TEXTMANAGER_H

#include <glm.hpp>
#include <memory>
#include <string>

class Shader;

/**
 * @brief Lightweight text rendering manager backed by FreeType glyph textures.
 */
class TextManager
{
public:
    /** @brief Construct text renderer state. */
    TextManager();
    /** @brief Release text rendering resources. */
    ~TextManager();

    /** @brief Initialize font glyph atlas and text shader. */
    bool init(const std::string &fontPath, const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
    /** @brief Render a single line of text in screen-space coordinates. */
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
    /** ASCII glyph table used by the text draw path. */
    GlyphData glyphs[128];
    bool initialized;
};

#endif // TEXTMANAGER_H
