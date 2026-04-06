#include "TextManager.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>

#include "Shader.h"

TextManager::TextManager() : vao(0), vbo(0), initialized(false)
{
}

TextManager::~TextManager()
{
    if (glfwGetCurrentContext() != nullptr)
    {
        if (vao != 0)
            glDeleteVertexArrays(1, &vao);
        if (vbo != 0)
            glDeleteBuffers(1, &vbo);
    }
    shader.reset();
}

bool TextManager::init(const std::string &fontPath, const std::string &vertexShaderPath, const std::string &fragmentShaderPath)
{
    shader = std::make_unique<Shader>(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

    // FreeType glyph setup
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "Failed to initialize FreeType" << std::endl;
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        std::cerr << "Failed to load font: " << fontPath << std::endl;
        FT_Done_FreeType(ft);
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "Failed to load glyph: " << (int)c << std::endl;
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows,
                     0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glyphs[c].textureId = texture;
        glyphs[c].size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
        glyphs[c].bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        glyphs[c].advance = face->glyph->advance.x >> 6;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Quad VAO/VBO for glyph rendering
    if (vao == 0)
        glGenVertexArrays(1, &vao);
    if (vbo == 0)
        glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    initialized = true;
    return true;
}

void TextManager::renderText(const std::string &text, float x, float y, float scale, const glm::vec3 &color)
{
    if (!initialized)
        return;

    shader->use();

    glm::mat4 projection = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f);
    shader->setMat4("projection", projection);
    shader->setVec3("textColor", color);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);

    for (char c : text)
    {
        if (c < 0 || c >= 128)
            continue;

        const GlyphData &glyph = glyphs[(unsigned char)c];

        float xpos = x + glyph.bearing.x * scale;
        float ypos = y - (glyph.size.y - glyph.bearing.y) * scale;

        float w = glyph.size.x * scale;
        float h = glyph.size.y * scale;

        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};

        glBindTexture(GL_TEXTURE_2D, glyph.textureId);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += glyph.advance * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
